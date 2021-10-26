/* 
   Author: Ricardo Sato 
   program based on the code of SUEmulator made by Fernando Contreras 
   
   This part of the code manage basically the protocol of data transfered
   between:
   * UB and radio;
   * BSU and CDAS;
   * BSU and NetMon.
*/

#ifndef comms_protocol
#define comms_protocol

#include <stdint.h>

struct stream_info
{
  int preamble_len;
  unsigned char preamble[20];

  unsigned char end;
};

struct msg_ctrl{
  int st;
  struct stream_info info;

  int i_preamble;
  unsigned char data[256];
  int i_data;
  uint32_t data_len;
  uint32_t min_data_len,max_data_len;
} ;
struct pack{
  unsigned char len; /* it include the additional 4 bytes; */
  unsigned char type;
  unsigned char data[256]; 
};



struct stream_info init_mess_str(unsigned char preamble[],int preamble_len);
int gen_message(unsigned char *buff,unsigned char type,
		unsigned char *add_info,int n_add_info,
		unsigned char *data,int datalen,
		struct stream_info msg);

struct msg_ctrl init_stream2msg(unsigned char *preamble,int preamble_len);

int stream2msg(const int Length, const unsigned char * Data, 
	       struct msg_ctrl *msg,struct pack *msgs );

#endif
