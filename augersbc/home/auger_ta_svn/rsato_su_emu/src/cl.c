#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sockets.h"
main(int argc,char *argv[])
{
  int sock,n,aux;
  unsigned char msg[102];

  
  if(argc>1){
    n=strlen(argv[1]);
    if(n>100)
      n=100;
    
    if(argv[1][0]=='c'){
      if(argc<2){
	printf("use as: %s c 'command ...'\n",argv[0]);
	exit(0);
      }
      n=strlen(argv[2]);
      msg[0]=3+n; /*3 is for: 
		    1 for the len,  
		    1 for the command and
		    1 for additional space
		  */
      msg[1]='c';
      msg[2]=' ';
      memcpy(msg+3,argv[2],n);
      n+=3;
    } else if(argv[1][0]=='T'){
      if(argc<5){
	printf("use as: %s T <GPS sec> <micro second> <window> \n");
      } else {
	n=14;
	msg[0]=n;
	msg[1]='T';
	aux=atoi(argv[2]);
	memcpy(msg+2,&aux,4);
	aux=atoi(argv[3]);
	memcpy(msg+6,&aux,4);
	aux=atoi(argv[4]);
	memcpy(msg+10,&aux,4);
      }
    } else {
      n=2;
      msg[0]=n;
      msg[1]=argv[1][0];
    }

    sock=make_socket_client ("localhost",20000);
    if(sock>0){
      write(sock,msg,msg[0]);
      close(sock);
    } else {
      printf("Not possible to open the socket ...\n");
    }
  } else {
    printf("use as: %s <cmd> <compl> \n"
	   "<cmd> would be: r,R,s,t,T,S\n"
	   "r - reboot station;\n"
	   "R - reboot station in broadcast mode \n"
	   "s - start acquition - start send T2\n"
	   "t - T3 request in random mode\n"
	   "T - T3 request - the complement would be gps second, microseconds and window\n"
	   "S - request stop the cdas emulator \n"
	   "c - send a command, <compl> would be the OS9 command which will be sent to UB\n",argv[0]);
    exit(0);
  }
}

