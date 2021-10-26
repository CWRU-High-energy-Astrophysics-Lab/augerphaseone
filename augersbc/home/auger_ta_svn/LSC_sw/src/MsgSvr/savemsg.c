/*******************************************

  $Author: guglielmi $
  $Date: 2011-08-18 16:34:03 -0400 (Thu, 18 Aug 2011) $
  $Revision: 1427 $

********************************************/
/*
  Description
   Save messages to CDAS into LinkLists, one per Priority.


  **********

  History

  V1 - guglielm - 2009/11/02 Creation

*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

/**
 * @defgroup save_msg  Saving messages waiting to be processed
 * @ingroup msgsvr
 *
 */
/**@{*/

#include "logfile.h"
#include "gpsstatus.h"
#include "linklib.h"
#include "msgsvr.h"
#include "central_local.h"
#include "savemsg.h"

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

static TO_CDAS_PKT CdasPkt ;

/**************************************************
  Global Variables
***************************************************/
extern GPS_STATUS * GpsStatus ;

/**************************************************
  Static Functions
***************************************************/

static SAVED_MESSAGE * create_saved( MSGSVR_PKT * the_msg, int size )
{
  SAVED_MESSAGE * new ;
  int saved_size, cdas_size ;
  TO_CDAS_MESSAGE * cdas_msg = (TO_CDAS_MESSAGE *)the_msg->payload ;

  cdas_size =  size - sizeof( the_msg->type ) ;
  LogPrint( LOG_INFO, "Cdas Size: %d - Cdas Type: %d\n",
	    cdas_size, cdas_msg->to_cdas_type ) ;

  saved_size = sizeof( SAVED_MESSAGE_HEADER ) + cdas_size ;
  LogPrint( LOG_INFO, "Saved Size: %d\n", saved_size ) ;

  new = LinkCreate( saved_size ) ;
  memcpy( new->payload, cdas_msg, cdas_size ) ;
  new->header.size = cdas_size ;
  new->header.left = cdas_size ;

  return new ;
}

static void add_to_high_priority( MSGSVR_PKT * the_msg, int size )
{
  SAVED_MESSAGE * new ;

  /* Create the SAVED_MESSAGE */
  new = create_saved( the_msg, size ) ;

  LinkAddTail( new, PFIRST_HIGH, PLAST_HIGH ) ;
  NbHigh++ ;
}

static void add_to_medium_priority( MSGSVR_PKT * the_msg, int size )
{
  SAVED_MESSAGE * new ;

  /* Create the SAVED_MESSAGE */
  new = create_saved( the_msg, size ) ;

  LinkAddTail( new, PFIRST_MEDIUM, PLAST_MEDIUM ) ;
  NbMedium++ ;
}

static void add_to_low_priority( MSGSVR_PKT * the_msg, int size )
{
  SAVED_MESSAGE * new ;

  /* Create the SAVED_MESSAGE */
  new = create_saved( the_msg, size ) ;

  LinkAddTail( new, PFIRST_LOW, PLAST_LOW ) ;
  NbLow++ ;
}

/**************************************************
  Global Functions
***************************************************/

int SavedMessageSave( MSGSVR_PKT * the_msg, int size )
{
  switch( the_msg->type ) {
  case HIGH_PRIORITY:
    add_to_high_priority( the_msg, size ) ;
    break ;
  case MEDIUM_PRIORITY:
    add_to_medium_priority( the_msg, size ) ;
    break ;
  case LOW_PRIORITY:
    add_to_low_priority( the_msg, size ) ;
    break ;
  }
  return NbHigh + NbMedium + NbLow ;
}

int SavedMessageLeft()
{
  return NbHigh + NbMedium + NbLow ;
}

#if 0

/* OBSOLETE */
/* Build the final packet to CDAS:
   2400 bits ( 300 Bytes )
   Fill first with HIGH_PRIORITY
    if ( room left ) fill with medium priority
     if ( room left ) fill with LOW PRIORITY
   send the CDAS PACKET
*/
void SavedMessageSend()
{
  int pkt_fill ;

  /* Prepare header of CDAS_PACKET:
     size, LSID, nb msg
  */
  CdasPkt.header.lsid = GpsStatus->LsId ;
  CdasPkt.header.length = 0 ;
  CdasPkt.header.nb_msg = 0 ;
  memset( CdasPkt.payload, 0, TO_CDAS_PKT_PAYLOAD_SIZE ) ;
  pkt_fill = 0 ;

  /* Any HIGH PRIORITY message ? */
  if ( NbHigh != 0 ) {
    SAVED_MESSAGE * cur = NULL ;

    LogPrintSysDate( LOG_INFO,
		     "Add %d High Priority Message(s) to TO_CDAS_PKT\n",
		     NbHigh ) ;
    for( cur = FirstHigh ; cur != NULL ; ) {
      /* Get the HIGH Message
	 copy payload to CDAS PKT (TODO check if size is OK)
      */
      if ( Verbose ) LogPrint( LOG_INFO, "cur->header.size: %d\n",
			       cur->header.size ) ;
      memcpy( &CdasPkt.payload[pkt_fill], cur->payload, cur->header.size ) ;
      cur->header.left = 0 ;
      pkt_fill += cur->header.size ;
      CdasPkt.header.length += cur->header.size ;
      CdasPkt.header.nb_msg++ ;

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
  }

  LogPrint( LOG_INFO, "Pkt Length: %d, Nb Msg: %d, LSID: %04X\n",
	    CdasPkt.header.length, CdasPkt.header.nb_msg,
	    CdasPkt.header.lsid ) ;
  LogPrint( LOG_INFO, "Left High Priority: %d\n", NbHigh ) ;

  /* Actually send the message to Radio */
  CanlibSendDataMsg( &CdasPkt, TO_CDAS_PKT_SIZE ) ;
}
#endif

void SavedMessageReset()
{
  SAVED_MESSAGE * cur, * old, * next ;

  if ( NbHigh != 0 )
    for( cur = FirstHigh ; cur != NULL ; ) {
      /* delete all messages in the queue */
      next = LinkNext( cur ) ;
      old = LinkUnlink( cur ) ;
      LinkDelete( old ) ;
      cur = next ;
      NbHigh-- ;
    }

  if ( NbMedium != 0 )
    for( cur = FirstMedium ; cur !- NULL ; ) {
      /* delete all messages in the queue */
      next = LinkNext( cur ) ;
      old = LinkUnlink( cur ) ;
      LinkDelete( old ) ;
      cur = next ;
      NbMedium-- ;
    }

  if ( NbLow != 0 )
    for( cur = FirstLow ; cur !- NULL ; ) {
      /* delete all messages in the queue */
      next = LinkNext( cur ) ;
      old = LinkUnlink( cur ) ;
      LinkDelete( old ) ;
      cur = next ;
      NbLow-- ;
    }

}

/**@}*/
