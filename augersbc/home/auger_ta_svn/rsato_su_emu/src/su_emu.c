#include "ub_side.h"
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#include <sys/stat.h>    
#include <fcntl.h>       

#include <stdio.h>
#include <stdlib.h>
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

int main(int argc,char *argv[])
{
  struct ub_io_ctrl ub;
  struct pack packs[30];
  int np,i,j;
  fd_set inputs;
  
  struct timeval timeout;
  
  printf("start ...\n");

  if(argc<4 || 
     (argc==2 && 
      (strcmp(argv[1],"-h")==0 ||
       strcmp(argv[1],"-?")==0 ||
       strcmp(argv[1],"-help")==0 ||
       strcmp(argv[1],"--help")==0)))
    use_as(argv[0]);
  
  i=atoi(argv[3]);
  ub.fd_in=open(argv[1],O_RDONLY);
  ub.fd_out=open(argv[2],O_WRONLY);
  printf(".... %d %d\n",ub.fd_in,ub.fd_out);
  if(isatty(ub.fd_in) && isatty(ub.fd_out)){
    close(ub.fd_in);
    close(ub.fd_out);    
    ub=ub_io_init(argv[1],3000,B38400); /* device and radio id */ 
  } else {
    ub=ub_io_init_fifo(argv[1],argv[2],3000); /* device and radio id */
  } 
  FD_ZERO(&inputs);
  FD_SET (ub.fd_in, &inputs);
  timeout.tv_sec=0;
  timeout.tv_usec=0;
  
  while (select(FD_SETSIZE, &inputs, NULL, NULL, &timeout)>=0){
    printf("Getting input ...\n");
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    if (FD_ISSET (ub.fd_in, &inputs)) {
      printf("goin go read ...\n");
      np=ub_read_data(&ub,packs);
      if(np<0){
	close(ub.fd_in);
      }
      for(i=0;i<np;i++){
	printf("UB packet to foreward: %d %d \n",packs[i].len,packs[i].data[0]);
	for(j=0;j<packs[i].len;j++){
	  printf("%02x ",packs[i].data[j]);
	}
	printf("\n");
      }
    }
    FD_ZERO(&inputs);
    FD_SET (ub.fd_in, &inputs);
  }
}
