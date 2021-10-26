/*******************************************

  $Author: guglielmi $
  $Date: 2011-08-26 16:52:58 +0200 (Fri, 26 Aug 2011) $
  $Revision: 1451 $

********************************************/
/*
  Description
   Save messages to CDAS into LinkLists, one per Priority.


  **********

  History

  V1 - guglielm - 2009/11/02 Creation

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
#include <netinet/in.h>

#include "can.h"

#include "msgsvr_version.h"
#include "logfile.h"
#include "gpsutil.h"
#include "gpsstatus.h"
#include "svrconfig.h"
#include "linklib.h"
#include "msgsvr.h"
#include "central_local.h"
#include "candefs.h"
#include "canlib.h"
#include "tocdas.h"
#include "acklist.h"
#include "memlib.h"

/**************************************************
  Static Variables
***************************************************/

static SAVED_MESSAGE * FirstHigh = NULL, * LastHigh = NULL,
  * FirstMedium = NULL, * LastMedium = NULL,
  * FirstLow = NULL, * LastLow = NULL ;

static int NbHigh = 0, NbMedium = 0, NbLow = 0 ;

#define PFIRST_HIGH (void **)&FirstHigh
#define PLAST_HIGH  (void **)&LastHigh
#define PFIRST_MEDIUM (void **)&FirstMedium
#define PLAST_MEDIUM  (void **)&LastMedium
#define PFIRST_LOW (void **)&FirstLow
#define PLAST_LOW  (void **)&LastLow

static TO_CDAS_PKT * CdasPkt = NULL ;

static int PktNumber = 0 ;

static int MsgNb = 1 ;

/**************************************************
  Global Variables
***************************************************/
extern GPS_STATUS * GpsStatus ;

extern SVR_CONFIG SvrConfig ;
extern MSGSVR_STATUS * SvrStatus ;

extern int Verbose ;

/**************************************************
  Static Functions
***************************************************/

static SAVED_MESSAGE * create_saved( MSGSVR_PKT * pkt )
{
  SAVED_MESSAGE * new ;
  int saved_size, cdas_size ;
  TO_CDAS_MESSAGE * cdas_msg = (TO_CDAS_MESSAGE *)pkt->payload ;
  int size ;

  size = pkt->header.length ;

  cdas_size = size - sizeof( TO_CDAS_MESSAGE_HEADER ) ;
  if ( Verbose )
    LogPrint( LOG_INFO,
		     "Save Cdas Msg: Msgpkt size: %d,Cdas Msg Size: %d - Cdas Type: %d\n",
		     size, cdas_size, cdas_msg->header.type ) ;

  saved_size = sizeof( SAVED_MESSAGE_HEADER ) + cdas_size ;
  if ( Verbose ) LogPrint( LOG_INFO, "Saved Size: %d\n", saved_size ) ;

  new = LinkCreate( saved_size ) ;
  memcpy( new->payload, cdas_msg->payload, cdas_size ) ;
  new->header.type = cdas_msg->header.type ;
  new->header.size = cdas_size ;
  new->header.left = cdas_size ;
  new->header.pos = 0 ;
  new->header.slice = 0 ;
  new->header.msg_nb = MsgNb << 2 ;
  MsgNb++ ;

  return new ;
}

static void add_to_high_priority( MSGSVR_PKT * pkt )
{
  SAVED_MESSAGE * new ;

  /* Create the SAVED_MESSAGE */
  new = create_saved( pkt ) ;

  LinkAddTail( new, PFIRST_HIGH, PLAST_HIGH ) ;
  NbHigh++ ;
}

static void add_to_medium_priority( MSGSVR_PKT * pkt )
{
  SAVED_MESSAGE * new ;

  /* Create the SAVED_MESSAGE */
  new = create_saved( pkt ) ;

  LinkAddTail( new, PFIRST_MEDIUM, PLAST_MEDIUM ) ;
  NbMedium++ ;
}

static void add_to_low_priority( MSGSVR_PKT * pkt )
{
  SAVED_MESSAGE * new ;

  /* Create the SAVED_MESSAGE */
  new = create_saved( pkt ) ;

  LinkAddTail( new, PFIRST_LOW, PLAST_LOW ) ;
  NbLow++ ;
}

/**************************************************
  Global Functions
***************************************************/

int ToCdasSave( MSGSVR_PKT * pkt )
{
  switch( pkt->header.type ) {
  case HIGH_PRIORITY:
    if ( Verbose ) LogPrint( LOG_INFO, "ToCdasSave - add_to_high_priority\n" ) ;
    add_to_high_priority( pkt ) ;
    break ;
  case MEDIUM_PRIORITY:
    if ( Verbose ) LogPrint( LOG_INFO, "ToCdasSave - add_to_medium_priority\n" ) ;
    add_to_medium_priority( pkt ) ;
    break ;
  case LOW_PRIORITY:
    if ( Verbose ) LogPrint( LOG_INFO, "ToCdasSave - add_to_low_priority\n" ) ;
    add_to_low_priority( pkt ) ;
    break ;
  }
  return NbHigh + NbMedium + NbLow ;
}

int ToCdasLeft()
{
  return NbHigh + NbMedium + NbLow ;
}

static void dump_pkt( TO_CDAS_PKT * pkt )
{
  int n , i ;
  char * str ;
  char s[8] ;
  unsigned char * buf = (unsigned char *)pkt ;

  str = mem_malloc( 4096 ) ;
  n = ntohs( pkt->length ) + 2 ;
  sprintf( str, "CdasPkt Dump: Length: %d, Msg #: %d, Lsid: %d (%d), nmsg: %d",
	   n, pkt->pkt_nb, ntohs( pkt->lsid ), SvrConfig.LsId, pkt->nb_msg ) ;
  LogPrint( LOG_INFO, "%s\n", str ) ;
  LogPrint( LOG_INFO, "CdasPkt Dump 1: LsId = %d\n", SvrConfig.LsId ) ;

  for( i = 0 ; i<n ; ) {
    int k = i, m = i+8 ;

    if ( m >= n ) m = n ;
    *str = '\0' ;
    for( k = i ; k < m ; k++, i++, buf++ ) {
      sprintf( s, "%02x ", *buf ) ;
      strcat( str, s ) ;
    }
    LogPrint( LOG_INFO, "  --> %s\n", str ) ;
  }
  LogPrint( LOG_INFO, "CdasPkt Dump 2: LsId = %d\n", SvrConfig.LsId ) ;
  mem_free( str ) ;
}

static int pkt_fill,		/**< Position where to add a new msg */
  pkt_length,			/**< Actual total length of the pkt */
  pkt_left ;			/**< Room left in the pkt */

/** 
 * Add a message into the Cdas Packet. Possibly split the message into
 * several slices. Check if enough room left for a new message/slice.
 * 
 * @param cur Pointer to saved message waiting
 * 
 */
static void AddMsgToPkt( SAVED_MESSAGE * cur )
{
  /* Add msgsize/slice/type/msgnb */
  unsigned char *p_pkt, *p_pkt0 ;
  short ml = cur->header.left + MESSAGE_HEADER_LENGTH ; /* size of msg includng itself*/
  short dml ;
  unsigned char completion ;

  if ( Verbose )
    LOG_DEBUG( LOG_INFO, "cur->header.size: %d, left: %d (%d)\n",
	       cur->header.size,
	      cur->header.left, pkt_left ) ;

  if ( ml > pkt_left ) {
    /* Not enough room for the whole message */
    if ( pkt_left <= MESSAGE_HEADER_LENGTH ) {
      /* Not enough room for a slice */
      return ;
    }
    /* Add a slice */
    ml = pkt_left ;
    if ( cur->header.slice == 0 ) {
      completion = COMPLETION_FIRST ;
      if ( Verbose ) LogPrint( LOG_INFO, "Add a FIRST slice %d bytes\n", ml ) ;
    }
    else {
      completion = COMPLETION_NEXT ;
      if ( Verbose ) LogPrint( LOG_INFO, "Add a NEXT slice %d bytes\n", ml ) ;
    }
  }
  else if ( cur->header.slice == 0 ) completion = COMPLETION_ALL ;
  else {
    completion = COMPLETION_LAST ;
    if ( Verbose ) LogPrint( LOG_INFO, "Add a LAST slice %d bytes\n", ml ) ;
  }

  dml = ml - MESSAGE_HEADER_LENGTH ;
  if ( Verbose ) LogPrint( LOG_INFO, "Completion = 0x%02x\n", completion ) ;

  p_pkt0 = &CdasPkt->payload[pkt_fill] ;
  p_pkt = p_pkt0 ;
  p_pkt = short_to_bytes( p_pkt, ml ) ;

  pkt_fill += 2 ;
  *p_pkt++ = (cur->header.slice & SLICE_MASK) | completion ;
  cur->header.slice++ ;
  *p_pkt++ = cur->header.type ;
  *p_pkt++ = cur->header.msg_nb ;
  pkt_fill += 3 ;
  if ( Verbose )
   LogPrint( LOG_INFO, " msg size: %02x%02x, type: %d, number: %x\n",
	     *p_pkt0, *(p_pkt0+1), *(p_pkt0+3), *(p_pkt0+4) ) ;
  memcpy( p_pkt, &cur->payload[cur->header.pos],
	  dml ) ;
  cur->header.pos += dml ;
  cur->header.left -= dml ;
  pkt_fill += dml ;
  CdasPkt->length += ml ;
  CdasPkt->nb_msg++ ;
  CdasPkt->pkt_nb = PktNumber++ ;

  pkt_left = TO_CDAS_PKT_PAYLOAD_SIZE - pkt_fill ;
  if ( Verbose )
    LogPrint( LOG_INFO, "    Pkt Max Length: %d, Fill: %d, Left: %d\n",
	      TO_CDAS_PKT_PAYLOAD_SIZE, pkt_fill, pkt_left ) ;
  return ;
}

/* Build the final packet to CDAS:
   Maximum 2400 bits ( 300 Bytes )
   Fill first with HIGH_PRIORITY
    if ( room left ) fill with medium priority
     if ( room left ) fill with LOW PRIORITY
   send the CDAS PACKET
*/
void ToCdasSend()
{
  short id = SvrConfig.LsId ;

  /* Prepare header of CDAS_PACKET:
     size, LSID, nb msg
  */
  if ( Verbose )
    LogPrint( LOG_INFO, "ToCdasSend - LsId: %d (%d)\n", SvrConfig.LsId, id ) ;
  if ( CdasPkt == NULL ) {
    CdasPkt = mem_malloc( TO_CDAS_PKT_SIZE ) ;
    if ( Verbose )
      LOG_DEBUG( LOG_INFO, "CdasPkt size: %d\n", TO_CDAS_PKT_SIZE ) ;
  }
  CdasPkt->lsid = htons( id ) ;
  CdasPkt->length = sizeof( TO_CDAS_PKT_HEADER ) - 1 ;
  if ( Verbose )
    LogPrint( LOG_INFO, "ToCdasSend - CdasPkt base length: %d\n", CdasPkt->length ) ;
  CdasPkt->nb_msg = 0 ;
  CdasPkt->reserved = 0 ;
  //memset( CdasPkt->payload, 0, TO_CDAS_PKT_PAYLOAD_SIZE ) ;
  pkt_fill = 0 ;
  pkt_left = TO_CDAS_PKT_PAYLOAD_SIZE ;

  /* Any HIGH PRIORITY message ? */
  if ( NbHigh != 0 ) {
    SAVED_MESSAGE * cur = NULL ;

    if ( Verbose )
      LogPrint( LOG_INFO,
		       "ToCdasSend - Add %d High Priority Message(s) to TO_CDAS_PKT\n",
		       NbHigh ) ;
    for( cur = FirstHigh ; cur != NULL ; ) {
      /* Get the HIGH PRIORITY Message
	 copy payload to CDAS PKT (TODO check if size is OK)
      */
      if ( Verbose )
	LogPrint( LOG_INFO,
		  "ToCdasSend - cur->header.size: %d\n", cur->header.size ) ;
      if ( pkt_left > MESSAGE_HEADER_LENGTH ) {
	AddMsgToPkt( cur ) ;

	/* Delete message from queue if empty */
	if ( cur->header.left == 0 ) {
	  SAVED_MESSAGE * old, * next ;

	  next = LinkNext( cur ) ;
	  old = LinkUnlink( cur ) ;
	  LinkDelete( old ) ;
	  cur = next ;
	  NbHigh-- ;
	}
	else cur = LinkNext( cur ) ;
      }
      else break ;
    }
  }
  if ( NbMedium != 0 ) {
    /* Add MEDIUM PRIORITY messages (as much as possible) */
    SAVED_MESSAGE * cur = NULL ;

    if ( Verbose )
      LogPrint( LOG_INFO,
		       "ToCdasSend - Add %d MEDIUM PRIORITY Message(s) to TO_CDAS_PKT\n",
		       NbMedium ) ;
    for( cur = FirstMedium ; cur != NULL ; ) {
      /* Get the HIGH PRIORITY Message
	 copy payload to CDAS PKT (TODO check if size is OK)
      */
      if ( Verbose )
	LogPrint( LOG_INFO, 
		  "ToCdasSend - cur->header.left: %d\n", cur->header.left ) ;
      if ( pkt_left > MESSAGE_HEADER_LENGTH ) {
	AddMsgToPkt( cur ) ;

	/* Delete message from queue if empty */
	if ( cur->header.left == 0 ) {
	  SAVED_MESSAGE * old, * next ;

	  next = LinkNext( cur ) ;
	  old = LinkUnlink( cur ) ;
	  LinkDelete( old ) ;
	  cur = next ;
	  NbMedium-- ;
	}
	else cur = LinkNext( cur ) ;
      }
      else break ;
    }
  }
  if ( NbLow != 0 ) {
    /* Add LOW PRIORITY messages (as much as possible) */
    SAVED_MESSAGE * cur = NULL ;

    if ( Verbose )
      LogPrint( LOG_INFO,
		       "ToCdasSend - Add %d LOW PRIORITY Message(s) to TO_CDAS_PKT\n",
		       NbLow ) ;
    for( cur = FirstLow ; cur != NULL ; ) {
      /* Get the LOW PRIORITY Message
	 copy payload to CDAS PKT (TODO check if size is OK)
      */
      if ( Verbose ) LogPrint( LOG_INFO, "cur->header.left: %d\n", cur->header.left ) ;
      if ( pkt_left >  MESSAGE_HEADER_LENGTH ) {
	AddMsgToPkt( cur ) ;

	/* Delete message from queue if empty */
	if ( cur->header.left == 0 ) {
	  SAVED_MESSAGE * old, * next ;

	  next = LinkNext( cur ) ;
	  old = LinkUnlink( cur ) ;
	  LinkDelete( old ) ;
	  cur = next ;
	  NbLow-- ;
	}
	else cur = LinkNext( cur ) ;
      }
      else break ;
    }
  }

  if ( Verbose ) {
    LogPrint( LOG_INFO, "ToCdasSend - Pkt Length: %d, Nb Msg: %d, LSID: %d\n",
	      CdasPkt->length, CdasPkt->nb_msg,
	      ntohs(CdasPkt->lsid) ) ;
    LogPrint( LOG_INFO, 
	      "ToCdasSend - Left Messages: %d\n", NbHigh + NbMedium + NbLow ) ;
  }
  pkt_length = CdasPkt->length ;
  CdasPkt->length = htons( CdasPkt->length -2 ) ;
  if ( Verbose > 2 )
    dump_pkt( CdasPkt ) ;

  /* Actually send the message to Radio */
  /**
   * @todo Make sure that we get an ACK from LR. Needs timeout and
   * what to do in this case or ACK bad
   */
  AddAck() ;
  if ( Verbose ) LogPrint( LOG_INFO, "Any Ack: %d\n", AnyAck() ) ;

  int err ;
  
  err = CanlibSendDataMsg( DATA_COMMAND_UP_BEGIN, (void *)CdasPkt, pkt_length ) ;
  if ( err != 0 ) {
    LogPrint( LOG_INFO,
	      "ToCdasSend - Error sending message: %d\n     %s\n", errno,
	      strerror( errno ) ) ;
  }
  else {
    if ( Verbose ) {
      LogPrint( LOG_INFO,
		"ToCdasSend - Pkt Sent, LsId = %d\n", SvrConfig.LsId ) ;
    }

    SvrStatus->to_cdas.nb_pkt++ ;
    SvrStatus->to_cdas.tot_size += pkt_length ;
  }
}
/**@}*/
