/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-22 12:41:04 #$
  $Revision:: 1776             $

********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <arpa/inet.h>

/**
 * @defgroup handle_radio  Hande in and out specific radio messages
 * @ingroup msgsvr
 *
 * Handle incoming and outgoing messages form/to local radio.
 *
 */

/**@{*/
/**
 * @file   handle_radio.c
 * @author guglielm
 * @date   2011/04/01
 * 
 * @brief  Hande in and out specific radio messages
 *
 * Handle incoming and outgoing messages form/to local radio.
 * 
 *
 */



#include "can.h"

#include "msgsvr_version.h"
#include "logfile.h"
#include "gpsutil.h"
#include "msgsvr.h"
#include "svrconfig.h"
#include "gpsstatus.h"
#include "candefs.h"
#include "canlib.h"
#include "central_local.h"
#include "acklist.h"
#include "handle_radio.h"
#include "lrstatus.h"

/**************************************************
  Static Variables
***************************************************/

/**
 * @struct FRAME_TYPE
 * @brief Description of the Monitoring data in a Monitoring Data Frame
 *
 * The structure defines the position in the frame where the data should be
 * taken from.
 * One such structure is defined for each of the Monitoring Data frame.
 *   - frame_idx: index in the frame.data of the monitoring element
 *   - data_idx : index in the LrStatus->lr_data where to save the element
 *   - size     : size (in bytes) of the element
 *
 * If less than 6 monitoring data element in the frame, complete frame_idx
 * with -1
 */
typedef struct {
  int frame_idx[6],
    data_idx[6],
    size[6] ;
} FRAME_TYPE ;


static FRAME_TYPE CGDFrameType[2] = {
  /* Frame type 1 for CGD data */
  {{ 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5}, { 1, 1, 1, 1, 1, 1}},
  /* Frame type 2 ... */
  {{ 1, 2, 3, 4, 5, 6}, {6, 7, 8, 9, 10, 11}, { 1, 1, 1, 1, -1, -1}}
} ;

static FRAME_TYPE LRFrameType[6] = {
  /* Frames for the radio and RF performance and behaviour */
  {{ 1, 3, 4, 5, 6}, { 12, 13, 14, 15, 16}, {2, 1, 1, 1, 1}},
  {{ 1, 2, 3, 4, 5, 6}, { 17, 18, 19, 20, 21, 22}, { 1, 1, 1, 1, 1, -1}},
  {{ 1, 2, 3, 4, 5, 6},{ 23, 24, 25, 26, 27, 28}, { 1, 1, 1, 1, 1, 1}},
  {{ 1, 2, 3, 4, 5, 6}, {29, 30, 31, 32, 33, 34 }, { 1, 1, 1, 1, 1, 1}},
  {{ 1, 2, 3, 4, 5, 6}, {35, 36, 37, 38, 39, 40}, { 1, 1, 1, 1, -1, -1}},
  {{ 1, 2, 3, 4, 5, 6}, {41, 42, 43, 44, 45, 46}, { 1, 1, 1, 1, -1, -1}}
} ;

/**************************************************
  Global Variables
***************************************************/

extern SVR_CONFIG SvrConfig ;
extern MSGSVR_STATUS * SvrStatus ;
extern GPS_STATUS * GpsStatus ;

extern int Verbose ;

extern LR_STATUS * LrStatus ;
extern void wireless_change( int wstat ) ;

/**************************************************
  Static Functions
***************************************************/

static void save_frame( canmsg_t * frame, FRAME_TYPE * type )
{
  int i ;
  
  for ( i = 0 ; i<6 ; i++ ) {
    int j = type->frame_idx[i] ;
    int k = type->data_idx[i] ;

    if ( type->size[i] == 1 ) {
      LrStatus->data[k] = frame->data[j] ;
    }
    else if ( type->size[i] == 2 ) {
      LrStatus->data[k] = frame->data[j] | frame->data[j+1] ;
    }
    else /* size == -1 ---> reserved for future use */
      LrStatus->data[k] = 0 ;
  }
}

static void save_cdg_data( canmsg_t * frame )
{
  int f_idx = frame->data[0] ;

  save_frame( frame, &CGDFrameType[f_idx] ) ;

}

static void save_lr_data( canmsg_t * frame )
{
  int f_idx ;

  f_idx = frame->data[0] ;
  save_frame( frame, &LRFrameType[f_idx] ) ;
}

/**************************************************
  Global Functions
***************************************************/

extern void wireless_change( int ) ;

/** 
 * Send a single frame to the radio.
 * 
 * @param id Can ID
 * @param msg Content, one frame only
 * @param len Nb of bytes, CAN_PAYLOAD_LENGTH maximum
 */
void SendToRadio( unsigned short id, void * msg, int len )
{
  canmsg_t frame ;

  frame.id = id ;
  /* Padding */
  memset( frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
  memcpy( frame.data, msg, len ) ;
  CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
}

int CanFromRadio( canmsg_t * frame )
{
  int can_id ;

  can_id = frame->id ;
  if ( Verbose ) LogPrint( LOG_INFO, "Got Frame - Source: Radio\n" ) ;

  switch ( can_id ) {
  case SW_VERSION_REPLY:
    LogPrint( LOG_INFO, "LR Sw Version: %02d%02d-%02d-%02d %02d:%02d\n",
	      frame->data[5], frame->data[4], frame->data[3],
	      frame->data[2], frame->data[1], frame->data[0] ) ;
    break ;
  case UP_STREAM_ACK:
    {
      short tid, tlength ;
      tid = ntohs( bytes_to_short( frame->data ) ) ;
      tlength = ntohs( bytes_to_short( &frame->data[2] ) ) ;
      if ( AnyAck() == 0 )
	LogPrint( LOG_WARNING, "Unexpected UP ACK %x. %x, %x\n",
		  tid, tlength, frame->data[4] ) ;
      else {
	RemAck() ;
	if ( Verbose )
	  LogPrint( LOG_INFO, "UP ACK OK: %x, %d, %d\n",
		    tid, tlength, frame->data[4] ) ;
	if ( frame->data[4] == STREAM_BAD ) {
	  LogPrint( LOG_WARNING,
		    "UP ACK  Bad - ID: 0x%x, length: 0x%x, Err: 0x%x\n",
		    tid, tlength, frame->data[4] ) ;
	  /* Should re-send the pkt ?
	   if yes, how ? */
	}
      }
    }
    break ;
  case WIRELESS_NET_STATUS:
    /* Go check wireless status change */
    if ( Verbose ) LogPrint( LOG_INFO, "Wireless Status Received: %d\n",
			     frame->data[0] ) ;
    wireless_change( frame->data[0] ) ;
    break ;
  case GPS_POSITION_REQUEST:
    {
      /* Send back the Gps position */
      GPS_POSITION_REPLY_1_MSG msg1 ;
      msg1.northing = GpsStatus->position_utm.northing ;
      msg1.easting = GpsStatus->position_utm.easting ;
      SendToRadio( GPS_POSITION_REPLY_1, &msg1,
		   GPS_POSITION_REPLY_1_MSG_LENGTH ) ;
      GPS_POSITION_REPLY_2_MSG msg2 ;
      msg2.altitude = GpsStatus->position_mas.height ;
      if ( GpsStatus->traim_status.traim_solution == TRAIM_SOLUTION_OK )
	msg2.valid = 0xFF ;
      else msg2.valid = 0 ;
      SendToRadio( GPS_POSITION_REPLY_2, &msg2,
		   GPS_POSITION_REPLY_2_MSG_LENGTH ) ;
    }
    break ;
  case GPS_DATE_TIME_REQ:
    {
      /* Send Back GPS date (seconds) and status */
      GPS_DATE_REPLY_MSG msg ;
      msg.date = GpsStatus->utc_current_time ;
      if ( GpsStatus->traim_status.traim_solution == TRAIM_SOLUTION_OK )
	msg.valid = 0xFF ;
      else msg.valid = 0 ;
      SendToRadio( GPS_DATE_TIME_REPLY, &msg, GPS_DATE_REPLY_MSG_LENGTH ) ;

    }
    break ;
  case GPS_1PPS_STATUS_REQ:
    {
      PPS_STATUS_REPLY_MSG msg ;
      /* Send back GPS Status */
      if ( GpsStatus->traim_status.traim_solution == TRAIM_SOLUTION_OK )
	msg.valid = 0xFF ;
      else {
	msg.valid = 0 ;
	if ( Verbose )
	  LogPrint( LOG_INFO, "GPS Status Reply, TRAIM Solution = %d\n",
		    GpsStatus->traim_status.traim_solution ) ;
      }
      SendToRadio( GPS_1PPS_STATUS, &msg, PPS_STATUS_MSG_LENGTH ) ;
    }
    break ;
  case SW_VERSION_REQ:
    if ( (frame->data[0] & SOURCE_DEST_LS) != 0 ) {
      SVR_VERSION_REPLY_MSG msg ;
      /* Reply with services version */
      msg.version = SvrStatus->ServicesVersion ;
      memset( msg.dummy, 0, 3 ) ;
      msg.source = SOURCE_DEST_LS ;
      SendToRadio( SW_VERSION_REPLY, &msg, SVR_VERSION_REPLY_MSG_LENGTH ) ;
    }
    /* else, not for me, ignore */
    break ;
  case ECHO_REQ:
    if ( (frame->data[0] & SOURCE_DEST_LS) != 0 ) {
      /* Reply with ECHO_REPLY_DEST */
      ECHO_MSG msg ;
      msg.source = SOURCE_DEST_LS ;
      memcpy( msg.echo, &frame->data[1], frame->length - 1 ) ;
      SendToRadio( ECHO_REPLY_DEST, &msg, frame->length ) ;
    }
    /* else, not for me, ignore ! */
    break ;
  case HW_SERIAL_REQ:
    if ( (frame->data[0] & SOURCE_DEST_LS) != 0 ) {
      SVR_VERSION_REPLY_MSG msg ;
      /* Reply with services version */
      msg.version = SvrStatus->CpuNumber ;
      memset( msg.dummy, 0, 3 ) ;
      msg.source = SOURCE_DEST_LS ;
      SendToRadio( HW_SERIAL_REPLY, &msg, SVR_VERSION_REPLY_MSG_LENGTH ) ;
    }
    /* else, not for me, ignore */
    break ;
  case CAN_BUFF_STAT_REQ:
    if ( (frame->data[0] & SOURCE_DEST_LS) != 0 ) {
      CAN_BUFF_STAT_REPLY_MSG msg ;
      /* Reply with Number of buffers */
      msg.source = SOURCE_DEST_LS ;
      msg.nmsg = 0xFF ;
      SendToRadio( CAN_BUFF_STAT_REPLY, &msg, CAN_BUFF_STAT_REPLY_MSG_LENGTH ) ;
    }
    break ;
  case ROUTINE_MONITOR_DATA_LR:
    /* Monitoring data from LR. Radio and RF link perfomances.
       First byte: frame type 1 to 6
    */
    if ( frame->data[7] == (SOURCE_DEST_LR_CPU | SOURCE_DEST_LR_IOP) ) {
      if ( Verbose )
	LogPrint( LOG_INFO, "LR monit frame #%d\n", frame->data[0] ) ;
      LrStatus->last_update = GpsStatus->gps_current_time ;
      save_lr_data( frame ) ;
    }
    else if ( Verbose )
      LogPrint( LOG_INFO,
		"Bad LR Monit: %02x %02x %02x %02x %02x %02x %02x %02x\n",
		frame->data[0], frame->data[1], frame->data[2],
		frame->data[3], frame->data[4], frame->data[5],
		frame->data[6], frame->data[7]);
    break ;
  case ROUTINE_MONITOR_DATA_CGD:
    /* Monitoring data from CGD. Baseband board Voltages and Temperature.
       First Byte: frame type 1 and 2
    */
    if ( frame->data[7] == SOURCE_DEST_LR_CGD ) {
      if ( Verbose )
	LogPrint( LOG_INFO, "CGD monit frame #%d\n", frame->data[0] ) ;
      LrStatus->last_update = GpsStatus->gps_current_time ;
      save_cdg_data( frame ) ;
    }
    else if ( Verbose )
      LogPrint( LOG_INFO,
		"Bad CGD Monit: %02x %02x %02x %02x %02x %02x %02x %02x\n",
		frame->data[0], frame->data[1], frame->data[2],
		frame->data[3], frame->data[4], frame->data[5],
		frame->data[6], frame->data[7]);
    break ;
  }

  return 0 ;
}

/**@}*/
