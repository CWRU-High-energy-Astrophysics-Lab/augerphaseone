/*******************************************

  $Author: guglielmi $
  $Date: 2011-08-01 16:23:38 +0200 (Mon, 01 Aug 2011) $
  $Revision: 1364 $

********************************************/
/*
  Description



  **********

  History

  V1 - guglielm - 2009/07/28 Creation

*/
/**
 * @defgroup handle_can_msg  Can Messages Handling
 * @ingroup msgsvr
 *
 *
 */
/**@{*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <arpa/inet.h>

#include "can.h"

#include "logfile.h"
#include "gpsutil.h"
#include "msgqueuelib.h"
#include "svrconfig.h"
#include "msgsvr.h"
#include "gpsstatus.h"
#include "central_local.h"
#include "msgsvrclient.h"
#include "candefs.h"
#include "canlib.h"
#include "handle_radio.h"
#include "handle_msg.h"
#include "handle_can_msg.h"
#include "linklib.h"
#include "memlib.h"

/**************************************************
  Static Variables
***************************************************/

static unsigned char RadioFrame[RADIO_FRAME_LENGTH] ;
static int RadioFrameLength = 0, RadioFrameState = 0 ;
static FROM_CDAS_PKT * CdasMsg = NULL ;
static int CdasMsgLength = 0, CdasMsgPosition = 0 ;

/**************************************************
  Global Variables
***************************************************/

extern SVR_CONFIG SvrConfig ;
extern MSGSVR_STATUS * SvrStatus ;
extern GPS_STATUS * GpsStatus ;

extern int Verbose ;

/**************************************************
  Static Functions
***************************************************/

static void dump_input_pkt( unsigned char * buf, int n ) ;


static short GetLsId( unsigned char * pmsg )
{
  short lsid ;

  lsid = bytes_to_short( pmsg ) ;
  lsid &= 0x3FFF ;

  return lsid ;
}

static unsigned char * IsForMe( int * ok, int * ls_position, int * nb_ls )
{
  unsigned char * pmsg ;
  int dest_type ;
  unsigned short dest ;
  int count = 0 ;

  pmsg = CdasMsg->payload ;
  dest = bytes_to_short( pmsg ) ;
  dest_type = (dest & DESTINATION_MASK ) >> DESTINATION_SHIFT  ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Destination: 0x%02x%02x, type: %d, LsId: 0x%x\n",
	      *pmsg, *(pmsg+1), dest_type, GetLsId( pmsg ) ) ;

  if ( dest_type == BROADCAST_DESTINATION ) {
    *ok = 1 ;
    *ls_position = 0 ;
    *nb_ls = 0 ;
    return pmsg + 2 ;
  }
  else if ( dest_type == SINGLE_DESTINATION &&
	    GetLsId( pmsg ) == SvrConfig.LsId ) {
    *ok = 1 ;
    *ls_position = 1 ;
    *nb_ls = 1 ;
    return pmsg + 2 ;
  }
  else if ( dest_type == LIST_DESTINATION ) {
    /* Now browse the destination and check if mine */
    int i = 0, found = 0 ;
    short nbid ;

    nbid = bytes_to_short( pmsg ) ;
    nbid &= 0xFF ;
    LogPrint( LOG_INFO, "Destination = Liste, Nbid = %d\n", nbid ) ;
    pmsg += 2 ;
    for( i = 0 ; i < nbid && count < CdasMsgLength ;
	 i++, pmsg += 2, count += 2 ) {
      int id = bytes_to_short( pmsg ) ;

      LogPrint( LOG_INFO, "   ID = %d (0x%x)\n", id, id ) ;
      if ( id == SvrConfig.LsId ) {
	*ls_position = i + 1 ;
	found = 1 ;
      }
    }

    *nb_ls = nbid ;
    *ok = found ;
    return pmsg ;
  }
  else if ( dest_type == ANTILIST_DESTINATION ) {
    /* Now browse the destination and check if NOT mine */
    int i = 0, found = 1 ;
    short nbid ;

    nbid = bytes_to_short( pmsg ) ;
    nbid &= 0xFF ;
    LogPrint( LOG_INFO, "Destination = AntiListe, Nbid = %d\n", nbid ) ;
    pmsg += 2 ;
    for( i = 0 ; i < nbid && count < CdasMsgLength ;
	 i++, pmsg += 2, count += 2 ) {
      int id = bytes_to_short( pmsg ) ;

      LogPrint( LOG_INFO, "   ID = %d (0x%x)\n", id, id ) ;
      if ( id == SvrConfig.LsId ) found = 0 ;
    }
    *nb_ls = nbid ;
    *ls_position = 0 ;
    *ok = found ;
    return pmsg ;
  }
  return 0 ;
}

static 
void dump_input_pkt( unsigned char * buf, int n )
{
  char * str ;
  char s[8] ;
  int i ;

  LogPrint( LOG_INFO, "Dump Input Pkt\n" ) ;

  str = mem_malloc( 256 ) ;
  for ( i = 0 ; i < n ; ) {
    int k = i, m = i+8 ;

    if ( m >= n ) m = n ;
    *str = '\0' ;
    for( k = i ; k < m ; k++, i++, buf++ ) {
      sprintf( s, "%02x ", *buf ) ;
      strcat( str, s ) ;
    }
    LogPrint( LOG_INFO, "  --> %s\n", str ) ;
  }
  mem_free( str ) ;  
}

/** 
 * Pass the Cdas Message to proper recipient:
 *  - Check that it is for me.
 *  - Check that a client is registered.
 *  - Pass to the client.
 *
 * NOTE: There is only 1 message at a time from CDAS, thus the frame
 * does not contain a byte with the nb of messages.
 * 
 */
void DistributeCdasMsg()
{
  MSG_CLIENT_STRUCT * pcl = NULL ;
  MSGSVR_PKT pkt ;
  unsigned char * pmsg ;
  unsigned char cdas_type ;
  int ok ;
  int msg_length, ls_position, nb_ls ;

  pmsg = IsForMe( &ok, &ls_position, &nb_ls ) ;
  if ( ok != 1 ) {
    LogPrint( LOG_INFO, "Destination NOT for me, discard\n" ) ;
    return ;
  }

  /*
    Pas de nombre de messages. Si plusieurs messages, utiliser uniquement
    la taille de la frame
    nbmsg = *pmsg++ ;
    if ( Verbose )
      LOG_DEBUG( LOG_INFO, "Nb of msg: %d\n", nbmsg ) ;
  */
  /* TODO: Handle multiple messages */

  msg_length = bytes_to_short( pmsg ) ;
  cdas_type = *(pmsg + 3) ;
  if ( Verbose ) {
    LogPrint( LOG_INFO,
	      "DistributeCdasMsg: Message Length: %d, cdas type: %d\n",
	      msg_length, cdas_type ) ;
    LogPrint( LOG_INFO, "   Get Queue for cdas_type %d\n", cdas_type ) ;
  }
  if ( cdas_type == M_REBOOT ) {
    /* Special message: do a reboot of the LS */
    LogPrint( LOG_FATAL, "Got an M_REBOOT message\n" ) ;
    /* First Save the logfiles (in case)
       Wait till files are saved
       And finally reboot the LSC
    */
    /* Send frame to Radio */
    POWER_STATUS_SEND_MSG rd_msg ;
    rd_msg.source = SOURCE_DEST_LS ;
    rd_msg.valid = 0 ;
    SendToRadio( POWER_STATUS_SEND, &rd_msg, POWER_STATUS_SEND_MSG_LENGTH ) ;

    LogPrint( LOG_INFO, "Stop Acquisition\n" ) ;
    system( "das stop" ) ;
    LogPrint( LOG_INFO, "Reset High Voltage (just in case ...)\n" ) ;
    system( "sethv -D 0" ) ;
    LogPrint( LOG_INFO, "Keep LogFiles\n" ) ;
    system( "logkeep" ) ;
    LogPrint( LOG_INFO, "Now Reboot !\n" ) ;
    system( "reboot" ) ;
    return ;
  }
  else if ( cdas_type == M_CONFIG_TO_FLASH ) {
    /* Save config files */
    system( "saveconfig &" ) ;
    return ;
  }

  // send the message to appropriate recipent
  pcl = GetFirstClient() ;

  if ( pcl == NULL ) {
    LogPrint( LOG_WARNING, "Client Queue Empty\n" ) ;
    return ;
  }
  do {
    //  while( (pcl = GetClientQueue( cdas_type, first )) != NULL ) {
    
    int pkt_size = msg_length + sizeof(MSGSVR_PKT_HEADER) ;

    if ( IsGoodClient( cdas_type, pcl ) == 1 ) {
      if ( Verbose > 1 )
	printf( "Found Client %s [@%p], now prepare pkt(%d)\n",
		pcl->name, pcl, pkt_size ) ;
      pkt.header.type = MSGSVR_FROM_CDAS ;
      pkt.header.length = msg_length ;
      pkt.header.position = ls_position ;
      pkt.header.nb_ls = nb_ls ;
      if ( Verbose ) dump_input_pkt( pmsg, msg_length ) ;
      memcpy( pkt.payload, pmsg, msg_length ) ;

      /* The pkt size = message_length + type */
      if ( Verbose )
	LogPrint( LOG_INFO, "Sending %d bytes (payload: %d) to '%s'\n",
		  pkt_size, msg_length, pcl->name ) ;
      msgsnd( pcl->q_id, &pkt, pkt_size , IPC_NOWAIT ) ;
      SvrStatus->to_client.nb_pkt++ ;
      SvrStatus->to_client.tot_size += msg_length ;

    }
  } while ( (pcl = LinkNext( pcl )) != NULL ) ;


}

/** 
 * Receive a can frame of DATA_COMMAND_DOWN_BEGIN to DATA_COMMAND_DOWN_END.
 * Accumulate each frame into a "RadioFrame". At the end of the stream,
 * get the size of the CDAS Pkt and save into a CdasMessage.
 * 
 * Data Down streams are from CDAS, and the first 2 bytes contain the size
 * of the packet.
 * 
 * @param frame The can frame received
 * 
 * @return 0 if OK.
 */
static int CanFromCdas( canmsg_t * frame )
{
  int can_id ;
  static int NbRadioFrame = 0 ;

  can_id = frame->id ;
  if ( Verbose ) LOG_DEBUG( LOG_INFO, "Got CAN Frame - Source: CDAS\n" ) ;

  /* Fill the message */
  if ( can_id == DATA_COMMAND_DOWN_BEGIN && RadioFrameState == 0 ) {
    RadioFrameState = 1 ;
    RadioFrameLength = 0 ;
    NbRadioFrame = 1 ;
    if ( Verbose ) LogPrint( LOG_INFO, "Down Command Begin\n" ) ;
  }
  else if ( can_id == DATA_COMMAND_DOWN_END ) {
    if ( Verbose ) LogPrint( LOG_INFO, "Down Command End\n" ) ;
    // The end of the pkt from CDAS
    if ( RadioFrameState != 1 ) {
      LOG_DEBUG( LOG_WARNING, "Unexpected Down End\n" ) ;
      return 0 ;
    }
    RadioFrameState = 2 ;
  }
  else if ( can_id < DATA_COMMAND_DOWN_BEGIN ||
	    can_id > DATA_COMMAND_DOWN_MAX ) {
    /* Ignore */
    LogPrint( LOG_WARNING, "Got unexpected Can Msg, ID = 0x%02x\n", can_id ) ;
    return 0 ;
  }
  else NbRadioFrame++ ;

  if ( RadioFrameState != 2 ) {
    /*
      Concatenate the msg to RadioFrame
      Should check if the frame ID's are in the good order and no missing
      ID(s)
    */
    /*
      if ( can_id == (prev_id + 2) || (can_id == DATA_COMMAND_DOWN_BEGIN &&
           prev_id == DATA_COMMAND_DOWN_MAX) {
	It's OK
       else missing Id. So what ???

      By the way, what if i dont receive the ACK ?
    */
    memcpy( RadioFrame+RadioFrameLength, frame->data, CAN_PAYLOAD_LENGTH ) ;
    RadioFrameLength += CAN_PAYLOAD_LENGTH ;
    if ( Verbose )
      LogPrint( LOG_INFO, "Can frame #%d, Radio frame length: %d\n",
		NbRadioFrame, RadioFrameLength ) ;
    return 0 ;
  }

  /* Have got the last frame */
  /* Get the msg length (from the beginning of the received pkt)
     and malloc CdasMsg */
  if ( CdasMsgLength == 0 ) {
    /* First radio frame of the message */
    CdasMsgLength = bytes_to_short( RadioFrame ) ;
    SvrStatus->from_cdas.nb_pkt++ ;
    SvrStatus->from_cdas.tot_size += CdasMsgLength ;

    /* As the frames from CAN are always 8 bytes long, the last frame
       is (sometimes) greater than the actual length should be */
    CdasMsg = mem_malloc( CdasMsgLength + 16 ) ;
    CdasMsgPosition = 0 ;
    if ( Verbose )
      LogPrint( LOG_INFO, "First Radio Frame, Msg Length: %d (0x%x)\n",
		CdasMsgLength, CdasMsgLength ) ;
  }
  /* Add to CdasMsg */
  if ( Verbose ) {
    LogPrint( LOG_INFO, "Add Radio Frame #%d to Cdas Msg (length %d) at %d (0x%x)\n",
	      NbRadioFrame, CdasMsgLength, CdasMsgPosition, CdasMsgPosition ) ;
    LogPrint( LOG_INFO, "  Radio frame Length: %d\n", RadioFrameLength ) ;
  }
  unsigned char * ppp = (unsigned char *)CdasMsg ;
  memcpy( ppp + CdasMsgPosition, RadioFrame, RadioFrameLength ) ;
  CdasMsgPosition += RadioFrameLength ;

  /* Send Ack to radio */
  STREAM_ACK_MSG ack ;
  ack.id_to_ack = htons( (short)DOWN_STREAM_ACK ) ;
  ack.id_received = htons( (short)can_id ) ;
  short ln = (short)RadioFrameLength ;
  ack.length = htons( ln ) ;
  ack.status = STREAM_OK ;
  CanlibSendRadioMsg( (unsigned char *)&ack, STREAM_ACK_MSG_LENGTH ) ;
  if ( Verbose ) LogPrint( LOG_INFO, "Stream Ack sent\n" ) ;

  if ( Verbose )
    LogPrint( LOG_INFO, "Msg Length: %d, Position: %d, Radio frame length: %d\n",
	      CdasMsgLength, CdasMsgPosition, RadioFrameLength ) ;
  if ( CdasMsgPosition >= CdasMsgLength ) {
    /* End of the message, distribute to recipient */
    DistributeCdasMsg() ;

    mem_free( CdasMsg ) ;
    CdasMsgLength = 0 ;
    RadioFrameState = 0 ;
  }

  return 0 ;
}

static int CanFromNin( canmsg_t * frame )
{
  int can_id ;

  can_id = frame->id ;
  if ( Verbose ) LogPrint( LOG_INFO, "Got Frame - Source: NIN\n" ) ;
  /* Now handle the message
   How ? What kind of message is supposed to come from NIN ?
   Same as from CDAS ?
  */
  return 0 ;
}

/**************************************************
  Global Functions
***************************************************/

int HandleCanFrame( canmsg_t * frame )
{
  int can_id ;

  can_id = frame->id ;
  if ( Verbose ) LOG_DEBUG( LOG_INFO, "Got Id = %d [%x]\n", can_id, can_id ) ;

  if ( can_id < DATA_COMMAND_DOWN_BEGIN ||
       (can_id >= WIRELESS_NET_STATUS && can_id < TPCB_POWER_MEASURE ) ) {
    /* This is for the Radio itself */
    return CanFromRadio( frame ) ;
  }
  else if ( can_id >= DATA_COMMAND_DOWN_BEGIN &&
	    can_id <= DATA_COMMAND_DOWN_END ) {
    /* This is from CDAS
       We must concatenate the radio frames into a single cdas message.
       Send an ACK to radio after each radio frame.
    */
    return CanFromCdas( frame ) ;
  }
  else if ( can_id >= MAINT_COMMAND_DOWN_BEGIN &&
	    can_id <= MAINT_COMMAND_DOWN_END ) {
    /* This is for NIN */
    return CanFromNin( frame ) ;
  }
  else {
    if ( Verbose )
      LogPrint( LOG_WARNING, "Unexpected Can Id: %d [%x]\n", can_id, can_id ) ;
    return 0 ;
  }

}

/**@}*/
