#include "ub_side.h"
#include "msg_interpreter.h"
#include "t2.h"
#include "central_local.h"


#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#include <sys/stat.h>    
#include <fcntl.h>       

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENTS 5
#define REBOOT_PERIOD 600

void use_as(char *cmd)
{
  printf("Use as: \n"
	 "%s <fifo_in> <fifo_out> <radio id>\n"
	 "<fifo_in>, <fifo_out> input and output fifo  \n"
	 "<radio id> Id of the radio \n"
	 " if <fifo_in> and <fifo_out> are the device which controls \n"
	 " the communication with the UB, it would reopen the devices as \n"
	 " serial terminal ports \n"
	 "example:\n"
	 "%s FIFO/comm_in FIFO/comm_out 3000 \n",cmd,cmd);
  exit(0);
}

int send_cmd(struct ub_io_ctrl *ub,unsigned char *msg_in,int stid)
{
  struct pack pp;
  int i;
  int p;
  printf("send_cmd .... %x\n",stid);
  pp.type='D';
  pp.data[0]=0;
  pp.data[1]=0;
  pp.data[2]=0;
  pp.data[3]=0;
  if(stid==-1){
    /* it is to send message in broadcast mode */
    pp.data[4]=3<<6; /*broadcast mode */
    p=5;
  } else {
    pp.data[4]=1<<6; /*dest - station list */
    pp.data[5]=1; /*for only one station */

    pp.data[6]=((stid/256)&0xFF); /*station Id */
    pp.data[7]=stid%256;
    p=8;
  }
  pp.data[p+1]=0;
  pp.data[p+3]=0;
  if(msg_in[1]=='r' || msg_in[1]=='s' || msg_in[1]=='w'){
    pp.data[p]=3;
    pp.data[p+2]=((msg_in[1]=='r')?M_REBOOT:
		 (msg_in[1]=='s')?M_RUN_START_REQ:
		 M_WAKEUP);
  } else {
    if(msg_in[1]=='t' || msg_in[1]=='c'){
      pp.data[p+2]=(msg_in[1]=='t')?M_T3_YES:M_OS9_CMD;
      pp.data[p]=msg_in[0]-2+4; /*-2 because the msg_in have the type
				  and length, and +3 is because the UB
				  schematics consider slice, ...
				*/
      memcpy(pp.data+12,msg_in+2,pp.data[p]-3);
    }
  }
  
  pp.len=msg_in[0]-2+13+1; /*consider the "header" ... */
  
  for(i=0;i<pp.len;i++){
    printf("%02x ",pp.data[i]);
  }
  printf("\n");
  ub_send_packet(ub,pp);
  return(0);
}


int data_from_ub(struct ub_io_ctrl *ub,struct cdas_emu_str *cdas,  
		 struct cdas_emu_slice_str *msg)
{
  struct pack packs[30];
  struct t2list *t2s;
  int np,i,j;
  unsigned char dt[150];
  int nd;

  /*printf("=== data from ub === \n"); */

  np=ub_read_data(ub,packs);
  if(np<0){
    return(-1);
  }
  for(i=0;i<np;i++){
    printf("=======================\n"
	   "UB packet to foreward: %d %d \n----------------\n",
	   packs[i].len,packs[i].data[0]);
    
    cdas_emu_add(cdas,packs[i].data,packs[i].len);
    do {
      nd=cdas_emu_get(cdas,dt);
      if(nd>0){
	if(cdas_emu_get_msg(cdas,dt,msg)>0){
	  printf("msg... %d %d \n",msg->type,msg->nb);
	  if(msg->type==0x05){ //T2
	    printf("T2 ...\n");
	    T2_add_new(&(cdas->t2s),msg->msg,msg->nb);
	    j=T2_get_from_list(&(cdas->t2s),0,&t2s);
	    if(cdas->nt3_to_req>0){
	      cdas->nt3_to_req--;
	      t3req_random(&(cdas->t3req),t2s,dt);
	      send_cmd(ub,dt,cdas->st_id);
	    }
	    if(j>0 && cdas->t2out!=NULL){
	      fprintf(cdas->t2out,"Sec,nt2,scaler: %d %d %d\n",
		      t2s->second,t2s->nt2,t2s->scaler);
	      for(j=0;j<t2s->nt2;j++){
		fprintf(cdas->t2out,"%d %d:%d\n",
			j,t2s->t2[j].tr_type,t2s->t2[j].usec);
	      }
	      fprintf(cdas->t2out,"---\n");
	    }
	  } else if(msg->type==0x06){ //T3 evt 
	    if(cdas->t3out!=NULL){
	      printf("Writting T3 event ...\n");
	      fprintf(cdas->t3out,"Event ...\n");
	      for(j=0;j<msg->nb;j++){
		fprintf(cdas->t3out,"%02x%s",msg->msg[j],(j%10==9)?"\n":" ");
	      }
	      fprintf(cdas->t3out,"\n----------\n");
	      fflush(cdas->t3out);
	    } else {
	      printf("Get T3 event, but not possible to write it ...\n");
	    }
	  } else {
	    printf("======= new message ...: %02x %d %d %d\n",
		   msg->type,msg->mess_id,msg->version,msg->nb);
	    for(j=0;j<msg->nb;j++){
	      printf("%02x%s",msg->msg[j],((j%10==9 || j+1==msg->nb)?"\n":" "));
	    }
	    printf("=================================\n");
	  }
	}
      }
    } while(nd>0);
  }
  return(0);
}



int main(int argc,char *argv[])
{
  struct ub_io_ctrl ub;
  struct cdas_emu_str cdas;
  fd_set in,in_active;
  int nsets;
  
  int serv_ctrl,cl[MAX_CLIENTS],aux;
  int i,j;
  char cl_msg[100],msg_out[120];
  int rid;
  
  struct timeval last_conn,now;

  struct cdas_emu_slice_str msg;

  struct timeval timeout;
  
  int sec,usec,win; /* to use for the T3 request */

  printf("start ...\n");


  cdas_emu_init(&cdas);
  msg.msg=(unsigned char *)malloc(102400);

  if(argc<4 || 
     (argc==2 && 
      (strcmp(argv[1],"-h")==0 ||
       strcmp(argv[1],"-?")==0 ||
       strcmp(argv[1],"-help")==0 ||
       strcmp(argv[1],"--help")==0)))
    use_as(argv[0]);
  

  rid=atoi(argv[3]);
  ub.fd_in=open(argv[1],O_RDONLY);
  ub.fd_out=open(argv[2],O_WRONLY);
  printf(".... %d %d\n",ub.fd_in,ub.fd_out);
  if(isatty(ub.fd_in) && isatty(ub.fd_out)){
    close(ub.fd_in);
    close(ub.fd_out);    
    ub=ub_io_init(argv[1],rid,B38400); /* device and radio id */ 
  } else {
    ub=ub_io_init_fifo(argv[1],argv[2],rid); /* device and radio id */
  } 

  serv_ctrl=make_socket_server ("localhost",20000);
  if(serv_ctrl<0){
    printf("Something went wrong while trying to start the localhost server\n");
    exit(1);
  }

  FD_ZERO(&in_active);
  FD_SET (ub.fd_in, &in_active);

  FD_SET(serv_ctrl,&in_active);
  timeout.tv_sec=5;
  timeout.tv_usec=0;
  for(i=0;i<MAX_CLIENTS;i++)
    cl[i]=0;

  in=in_active;

  /*sending a reboot at the first connection in broadcast mode */
  gettimeofday(&now,NULL);
  last_conn=now;

  while ((nsets=select(FD_SETSIZE, &in, NULL, NULL, &timeout))>=0){
    gettimeofday(&now,NULL);
    if(nsets>0){
      if (ub.fd_in>0 && FD_ISSET (ub.fd_in, &in)) {
	last_conn.tv_sec=now.tv_sec;
	last_conn.tv_usec=now.tv_usec;
	if(data_from_ub(&ub,&cdas,&msg)<0){
	  FD_CLR(ub.fd_in,&in_active);
	  close(ub.fd_in);
	  ub.fd_in=-1;
	}
      }
      
      if(FD_ISSET(serv_ctrl,&in)){
	j=-1;
	for(i=0;i<MAX_CLIENTS && j==-1;i++){
	  if(cl[i]==0){
	    j=i;
	  }
	}
	if(j>=0){
	  cl[j]=new_client(serv_ctrl);
	  if(cl[j]<0){
	    cl[j]=0;
	  } else {
	    FD_SET(cl[j],&in_active);
	  }
	}
      }
      for(i=0;i<MAX_CLIENTS ;i++){
	if(cl[i]>0){
	  printf("%d %d\n",i,cl[i]);
	  if(FD_ISSET(cl[i],&in)){
	    aux=read(cl[i],cl_msg,100);
	    printf("aux:%d\n",aux);
	    if(aux<=0){
	      FD_CLR(cl[i],&in_active);
	      close(cl[i]);
	      cl[i]=0;
	    } else {
	      if(cl_msg[1]=='S'){ /*stop ...*/
		for(i=0;i<MAX_CLIENTS;i++){
		  if(cl[i]>0)
		    close(cl[i]);
		}
		close(serv_ctrl);
		return(0);
	      } else {
		if(cl_msg[1]=='t'){
		  cdas.nt3_to_req++;
		  printf("Number of T3 to be requested: %d\n",cdas.nt3_to_req);
		} else if(cl_msg[1]=='T'){
		  memcpy(&sec,cl_msg+2,4);
		  memcpy(&usec,cl_msg+6,4);
		  memcpy(&win,cl_msg+10,4);
		  printf("T3 Request: sec,usec,win: %d %d %d\n",sec,usec,win);
		  t3req_generic(&(cdas.t3req),sec,usec,win,cl_msg);
		  send_cmd(&ub,cl_msg,cdas.st_id);
		} else if(cl_msg[1]=='R'){
		  /*send reboot in broadcast mode 
		    . it maybe usefull just after start the program */
		  cl_msg[0]=1;
		  cl_msg[1]='r';
		  last_conn.tv_sec=now.tv_sec;
		  send_cmd(&ub,cl_msg,-1); 
		} else if(cl_msg[1]=='w' || 
			  cl_msg[1]=='r' || 
			  cl_msg[1]=='s' || 
			  cl_msg[1]=='c')
		  send_cmd(&ub,cl_msg,cdas.st_id);
		else {
		  printf("Command requested look not valid %d\n",cl_msg[1]);
		}
	      } 
	    }
	  }
	}
      }
    } else {
      if(last_conn.tv_sec!=now.tv_sec)
	printf("time: %d %d\n",last_conn.tv_sec,now.tv_sec);
      if(last_conn.tv_sec+REBOOT_PERIOD<now.tv_sec){
	/*the last time the UB communicate was more than 10 minutes.
	  going to send a reboot in BROADCAST 
	*/
	printf("Sending reboot command. Station not answering for more than %d seconds\n",REBOOT_PERIOD);
	cl_msg[0]=1;
	cl_msg[1]='r';
	last_conn.tv_sec=now.tv_sec;
	send_cmd(&ub,cl_msg,-1); /*using -1 as station Id, 
				   will send in BROADCAST */
      }
    }
    in=in_active;
    
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    
  }
}
