#include "ub_side.h"
#include "pm_side.h"
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
	 "%s <Pm Address> <Pm Port> <BSU ID> <radio id> <serial port device>\n"
	 "\n<Pm Address> <Pm Port> - are the address and Port\n"
	 "<BSU ID> is the Id of BSU which would be recognized by Pm \n"
	 "<radio id> - it is just for the UB to identify the radio \n"
	 "<serial port device> - If it is not a serial port, one might use\n"
	 "     FIFO instead of serial port. In this case, the program will \n"
	 "     need one additional parameter which will corresponding to   \n"
	 "     the name of FIFO which will be the output for the radio  \n"
	 "     to program try to evaluate if the file is a tty type \n"
	 "examples:\n"
	 "%s 192.168.2.1 10600 70 3000 /dev/ttyUSB0\n"
	 "%s 192.168.2.1 10600 70 3000 FIFO/FROM_UB FIFO/TO_UB\n",
	 cmd,cmd,cmd);
  exit(0);
}

int main(int argc,char *argv[])
{
  struct ub_io_ctrl ub;
  struct pm_ctrl pm;
  struct pack packs[30];
  int np,i,j,fd;
  fd_set inputs;
  
  struct timeval timeout;
  
  if(argc<6 || 
     (argc==2 && 
      (strcmp(argv[1],"-h")==0 ||
       strcmp(argv[1],"-?")==0 ||
       strcmp(argv[1],"-help")==0 ||
       strcmp(argv[1],"--help")==0)))
    use_as(argv[0]);
  
  

  pm.ServerAddr=argv[1];
  pm.port=atoi(argv[2]);
  pm.BSUID=atoi(argv[3]);
  
  if(PM_conn_init(&pm)<0){
    printf("Some problem to connect with Pm:\n");
    printf("Check Pm address and port ... (addr,port)=(%s,%d)\n",
	   pm.ServerAddr,pm.port);
    return(1);
  }
  printf("PM Started ...\n");
  i=atoi(argv[4]);
  /*this is just to evaluate if it if a serial port or not */
  fd=open(argv[5],O_RDONLY | O_NONBLOCK);
  if(fd<0){
    printf("Not possible to open the file %s\n",argv[5]);
    use_as(argv[0]);    
  }
  j=isatty(fd);
  close(fd); /*it is going to reopen ... */

  if(j){
    ub=ub_io_init(argv[5],i,B38400); /* device and radio id */    
  } else {
    ub=ub_io_init_fifo(argv[5],argv[6],i);
  }
  
  printf("connection with UB ... %d %d\n",ub.fd_in,ub.fd_out);
  FD_ZERO(&inputs);
  FD_SET (ub.fd_in, &inputs);
  FD_SET (pm.socket, &inputs);
  timeout.tv_sec=1;
  timeout.tv_usec=0;
  
  while (select(FD_SETSIZE, &inputs, NULL, NULL, &timeout)>=0){
    printf("passing ...\n");
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    if (FD_ISSET (ub.fd_in, &inputs)) {
      np=ub_read_data(&ub,packs);
      if(np<0){
	close(ub.fd_in);
      }
      for(i=0;i<np;i++){
	printf("UB -> Pm: %d \n",packs[i].len);
	PM_Write(&pm,packs[i]);
      }
    }
    if (FD_ISSET (pm.socket, &inputs)) {
      np=PM_Read(&pm,packs);
      for(i=0;i<np;i++){
	printf("--->PM -> UB: %d \n",packs[i].len);
	ub_send_packet(&ub,packs[i]);
      }
    }
    FD_ZERO(&inputs);
    if(ub.fd_in<0 || ub.fd_out<0){
      ub_reopen(&ub);
    }
    FD_SET (ub.fd_in, &inputs);
    FD_SET (pm.socket, &inputs);
  }
}
