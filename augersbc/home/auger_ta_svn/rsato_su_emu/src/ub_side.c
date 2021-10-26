/* 
   Author: Ricardo Sato 
   program based on the code of SUEmulator made by Fernando Contreras 
   
   manage the connection with UB. Emulate radio of Auger

   The part of the code which emulate the radio protocol is just enough to
   establish the communication with the UB. It still need to implement
   most of the commands request/answer ...
*/

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "protocol.h"

#include "ub_side.h"


int ub_OpenSerialPort(char * TerminalPort,int baud) 
{
  /*call this function as:
    fd=OpenSerialPort("/dev/ttyS0",B38400)  
  */
  int fd;
  struct termios oldtio,newtio;
  
  fd = open(TerminalPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    printf("Error opening serial port!\n");
    exit(-1);
  }
  // Configure SerialPort
  tcgetattr(fd,&oldtio); // Keep the current config port.
  bzero(&newtio, sizeof(newtio)); // Clean for the new config.
  newtio.c_cflag = baud  | CS8 | CLOCAL | CREAD; //it is the radio configuration ...
  newtio.c_iflag = IGNPAR;// | ICRNL;
  newtio.c_oflag = 0;
  //newtio.c_lflag = ICANON;
  newtio.c_lflag =0;
  newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
  newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
  newtio.c_cc[VERASE]   = 0;     /* del */
  newtio.c_cc[VKILL]    = 0;     /* @ */
  newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
  newtio.c_cc[VTIME]    = 0;   /* temporizador entre caracter, no usado */
  newtio.c_cc[VMIN]     = 1;     /* bloqu.lectura hasta llegada de caracter. 1 */
  newtio.c_cc[VSWTC]    = 0;     /* '\0' */
  newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
  newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
  newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
  newtio.c_cc[VEOL]     = 0;     /* '\0' */
  newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
  newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
  newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
  newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
  newtio.c_cc[VEOL2]    = 0;     /* '\0' */
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);
  return fd;
}

void ub_side_reset(struct ub_io_ctrl *ctrl,int radio_id)
{
  int i;
  ctrl->in=init_stream2msg("  !LS2SU!",9);
  ctrl->out=init_mess_str("  !SU2LS!",9);
  ctrl->r_id=radio_id;
  ctrl->net_status=0x02; /*consider the network connected */
  ctrl->n_pack_send=0;
  for(i=0;i<256;i++){
    ctrl->pack_ack[i]=1; /* consider all previous packets as ack. */
  }
}
 

struct ub_io_ctrl ub_io_init(char *dev,int radio_id,int speed)
{
  int i;
  struct ub_io_ctrl ctrl;
  ctrl.fifo=0;
  ctrl.fd_in=ub_OpenSerialPort(dev,speed);
  ctrl.fd_out=ctrl.fd_in;
  ctrl.fname_out[0]=0;
  ub_side_reset(&ctrl,radio_id);
  return(ctrl);
}

struct ub_io_ctrl ub_io_init_fifo(char *dev_in,char *dev_out,int radio_id) 
{
  int i;
  struct ub_io_ctrl ctrl;
  ctrl.fifo=1;
  ctrl.fd_in=-1;
  ctrl.fd_out=-1;
  strcpy(ctrl.fname_in,dev_in);
  strcpy(ctrl.fname_out,dev_out);
  ub_reopen(&ctrl);
  printf("It have just opened ...\n");
  ub_side_reset(&ctrl,radio_id);
  return(ctrl);
}
void ub_reopen(struct ub_io_ctrl *ctrl)
{
  if(ctrl->fifo){
    if(ctrl->fd_in<0){
      ctrl->fd_in=open(ctrl->fname_in,O_RDONLY|O_NONBLOCK);
      printf("Input opened ... %d\n",ctrl->fd_in);
    }
    if(ctrl->fd_out<0){
      ctrl->fd_out=open(ctrl->fname_out,O_WRONLY|O_NONBLOCK);
      printf("Output opened ... %d\n",ctrl->fd_out);
    }
  }
}

void ub_senddata(struct ub_io_ctrl *ctrl,unsigned char type,
		 unsigned char *add_bytes,int add_bytes_size,
		 unsigned char *dat,int size)
{
  int nd,flag,i;
  unsigned char buff[300];
  nd=gen_message(buff,type,add_bytes,add_bytes_size,dat,size,ctrl->out);
  i=0;
  if(ctrl->fd_out>0){
    do{
      flag=write(ctrl->fd_out,buff+i,nd-i);
      if(flag>=0)
	i+=flag;
      else{
	/* close the output file descriptor because of error */
	close(ctrl->fd_out);
	ctrl->fd_out=-1;
	i=nd;
      }
    } while(i<nd && flag>0);
  } else {
    if(ctrl->fname_out[0]!=0){
      printf("try to open output ...");
      ctrl->fd_out=open(ctrl->fname_out,O_WRONLY | O_NONBLOCK);
      printf("%d ",ctrl->fd_out);
    }
  }
}

void ub_send_packet(struct ub_io_ctrl *ctrl,struct pack pp)
{
  unsigned char add_info[3];
  int n_add_info;
  int i;
  printf("ub_send_packet\n");
  if(! ctrl->pack_ack[ctrl->n_pack_send]){
    printf("An old packet have not an ack ... %d",ctrl->n_pack_send);
  }
  n_add_info=0;
  if(pp.type=='D'){
    add_info[0]=ctrl->r_id/256;
    add_info[1]=ctrl->r_id%256;
    add_info[2]=ctrl->n_pack_send;
    n_add_info=3;
  }
  ub_senddata(ctrl,pp.type,add_info,n_add_info,pp.data,pp.len);
  ctrl->pack_ack[ctrl->n_pack_send]=0;
  ctrl->n_pack_send++;  
}

int ub_read_data(struct ub_io_ctrl *ctrl,struct pack packs[])
{
  unsigned char buff[300],type;
  struct pack pp[30];
  int n,nd,i;

  nd=0;
  n=read(ctrl->fd_in,buff,300);
  if(n>0){
    n=stream2msg(n,buff,&(ctrl->in),pp);
    for(i=0;i<n;i++){
      type=pp[i].type;
      buff[0]=ctrl->r_id/256; /* using buff in this place as data message ...*/
      buff[1]=ctrl->r_id%256;
      buff[2]=pp[i].data[2]; /* packet number */
      if (type=='D'){ /* data command */
	/* 3: 2 bytes of Id and 1 byte of packet number */
	packs[nd].len=pp[i].len-3;
	packs[nd].type=pp[i].type;
	memcpy(packs[nd].data,pp[i].data+3,packs[nd].len); 
	nd++;
	ub_senddata(ctrl,'d',"",0,buff,3); /* data ack */
      } else if (type=='d'){
	ctrl->pack_ack[pp[i].data[2]]=1;
      } else if(type=='U'){ /* Id request */
	ub_senddata(ctrl,'u',"",0,buff,2); /*Id ack */
      } else 
	if(type=='N'){ /* Network Status Req */
	  printf("Sending network status: %d\n",ctrl->net_status);
	  buff[0]=ctrl->net_status;
	  ub_senddata(ctrl,'n',"",0,buff,1); /*network ack status*/	    
	} 
    } /* end of for loop */
  } else {
    return(-1);
  }
  return(nd);
}
