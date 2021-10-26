/* 
   Author: Ricardo Sato 
   program based on the code of SUEmulator made by Fernando Contreras 
   
   manage the connection with UB. Emulate radio of Auger

*/

#include <stdint.h>
#include "protocol.h"

struct ub_io_ctrl
{
  int fd_in,fd_out; /*it is to allow use of FIFO instead of serial port */
  int fifo;
  char fname_in[50],fname_out[50];
  struct stream_info out;
  struct msg_ctrl in;
  uint16_t r_id;
  unsigned char net_status; /* 0 -> attempting to join a network
			       1 -> found a network - trying to establish a 
			       .     data connection 
			       2 -> normal communication
			       3 -> lost connection */
  unsigned char n_pack_send;
  int pack_ack[256]; 
};
int ub_OpenSerialPort(char * TerminalPort,int baud);
struct ub_io_ctrl ub_io_init(char *dev,int radio_id,int speed);
struct ub_io_ctrl ub_io_init_fifo(char *dev_in,char *dev_out,int radio_id);

void ub_reopen(struct ub_io_ctrl *ctrl);
void ub_senddata(struct ub_io_ctrl *ctrl,unsigned char type,
	      unsigned char *add_bytes,int add_bytes_size,
	      unsigned char *dat,int size);
void ub_send_packet(struct ub_io_ctrl *ctrl,struct pack pp);
int ub_read_data(struct ub_io_ctrl *ctrl,struct pack packs[]);
