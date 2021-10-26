/* 
   Author: Ricardo Sato 
   program based on the code of SUEmulator made by Fernando Contreras 
   
   manage the connection with PM (emulate BSU <--> PM connection)
*/

#include <stdint.h>
#include "protocol.h"


/*===== packet information for connect with Pm === */
#define Gs_MESSAGE_LENGTH  0x28
#define Gs_VERSION         1
#define Gs_PmBSUID         1
#define Gs_HEADER_LENGTH   16
#define Gs_MESSAGE_VERSION 1
#define Gs_NO_OF_SERVICES  1
#define Gs_SERVICE_ID      1
#define Gs_NO_OF_PARAMS    4
#define Gs_TRAILER         0x1ABCDEF2


#define PM_RX_PREAMBLE         "!PC2BS!"	// preamble received from board
#define PM_RX_PREAMBLE_LENGTH  7
#define PM_TX_PREAMBLE		"  !BS2PC!"
#define PM_TX_PREAMBLE_LENGTH	9



struct pm_ctrl
{
  char *ServerAddr;
  int port;
  unsigned char BSUID;
  
  int socket;
  int nerr;
  struct stream_info out;
  struct msg_ctrl in;
};

void Pm_Negotiation(struct pm_ctrl *ctrl);
int PM_conn_init(struct pm_ctrl *ctrl);
int PM_Read(struct pm_ctrl *ctrl,struct pack *pp);
int PM_Write(struct pm_ctrl *ctrl,struct pack pp);
