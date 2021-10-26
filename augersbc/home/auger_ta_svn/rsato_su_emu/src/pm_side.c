/* 
   Author: Ricardo Sato 
   program based on the code of SUEmulator made by Fernando Contreras 
   
   manage the connection with PM (emulate BSU <--> PM connection)
*/

#include "pm_side.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/sem.h>
#include <errno.h>
#include <unistd.h>// Headers
#include <string.h>
#include <time.h>

void PM_negotiation(struct pm_ctrl *ctrl) {
  int32_t aux[10];
  int32_t n;
  
  aux[0]= htonl(Gs_MESSAGE_LENGTH);
  aux[1]= htonl(Gs_VERSION);
  aux[2]= htonl(Gs_PmBSUID);
  aux[3]= htonl(Gs_HEADER_LENGTH);
  aux[4]= htonl(Gs_MESSAGE_VERSION);
  aux[5]= htonl(Gs_NO_OF_SERVICES);
  aux[6]= htonl(Gs_SERVICE_ID);
  aux[7]= htonl(Gs_NO_OF_PARAMS);
  aux[8]= htonl((int32_t)ctrl->BSUID);
  aux[9]= htonl(Gs_TRAILER);
  n= write(ctrl->socket, (void*)aux,Gs_MESSAGE_LENGTH);

  if(n == Gs_MESSAGE_LENGTH) {
    printf("Correctly written Gs connection message\n");
  }
  else  {
    printf("Error : Unable to write Gs connection message\n");
  }
}

int PM_conn_init(struct pm_ctrl *ctrl)
{
  struct sockaddr_in MyConn;
  struct hostent *Host;
  Host = gethostbyname (ctrl->ServerAddr);
  if (Host == NULL) {
    printf ("Error getting hostname\n");
    ctrl->socket=-1;
    return -1;
  }
  MyConn.sin_family = AF_INET;
  MyConn.sin_addr.s_addr = ((struct in_addr*)(Host->h_addr))->s_addr;
  MyConn.sin_port = htons(ctrl->port);
  ctrl->socket = socket(AF_INET, SOCK_STREAM, 0);
  if (ctrl->socket == -1) {
    printf("I couldn't open TCP socket\n");
    return -1;
  }
  
  // connect to the server
  if (connect(ctrl->socket, (struct sockaddr *)&MyConn,sizeof (MyConn)) == -1){
    printf ("Error connecting to Pm\n");
    return -1;
  }
  if(fcntl(ctrl->socket, F_SETFL, O_NONBLOCK)) {
    printf("Failed to change socket type to non-blocking\n");
    return -1;
  }
  if(fcntl(ctrl->socket, F_SETFL, O_RDWR)) {
    printf("Failed to change socket type to read-write\n");
    return -1;
  }
  
  printf("Connected to the server: Doing Negotiation...\n");
  PM_negotiation(ctrl);
  /*to send to PM */
  ctrl->out=init_mess_str(PM_TX_PREAMBLE,PM_TX_PREAMBLE_LENGTH); 
  /* to receive from Pm */
  ctrl->in=init_stream2msg(PM_RX_PREAMBLE,PM_RX_PREAMBLE_LENGTH); 
  return ctrl->socket;
}


int PM_Read(struct pm_ctrl *ctrl,struct pack *pp)
{
  /* PREAMBLE, LENGTH(1), TYPE(1), BSU ID(1), DATA..., CRC(4), ETX(1)  */
  int nn;
  unsigned char buffer[256];
  int np,i,nd;
  nn = read(ctrl->socket, &buffer, 256);
  if (nn > 0) {
    ctrl->nerr=0;
    nn=stream2msg(nn, buffer,&(ctrl->in),pp);
    np=0;
    /* exclude BSU Id information and store only packets with type='D'*/
    for(i=0;i<nn;i++){
      if(pp[i].type=='D'){
	pp[np].type=pp[i].type;
	pp[np].len=pp[i].len-4; //4 is 1 of BSU id, 2 bytes (looks to be reserved for Id); 1 byte for counter
	memmove(pp[np].data,pp[i].data+4,pp[np].len);
	np++;
      }
    }
    return(np);
  }
  else if(nn == 0 || nn == -1) {
    ctrl->nerr++;
    if (ctrl->nerr == 10) {
      printf("There was a problem with the connection to Pm...\n");
      printf("Closing socket TCP...\n");
      close(ctrl->socket);
      printf("Done!\n");
      sleep(2);
      printf("Re-connecting...\n");
      do {
	sleep(2);
	PM_conn_init(ctrl);
      } while (ctrl->socket == -1);
      printf("Done!\n");
      ctrl->nerr=0;
    }
  }
  return(0);
}

int PM_Write(struct pm_ctrl *ctrl,struct pack pp)
{
  /* send the packet "pp" to the PM 
     remind that the pp.data have 4 reserved bytes at the begining
     which may be used to easier send some additional information.
     In this particular algorithms, it is sending the BSU Id.
     The data sent would have the following structure:
     PREAMBLE, LENGTH(1), TYPE(1), BSU ID(1), DATA..., CRC(4), ETX(1) 
     
  */
  unsigned char buff[300],add[10];
  int nd,nw,aux,i;
  add[0]=ctrl->BSUID;
  add[1]=0; /* 4 bytes looks to be not really used ... */
  add[2]=0;
  add[3]=0;
  add[4]=0;
  nd=gen_message(buff,pp.type,add,5,pp.data,pp.len,ctrl->out);
  nw=0;
  do{
    aux=write(ctrl->socket,buff+nw,nd-nw);
    if(aux>0)
      nw+=aux;
    if (nw!=nd){
      printf("It was not possible to write all requered data to PM: "
	     "%d written, %d needed to be written \n",nw,nd);
    }
  } while(nw<nd);
}

