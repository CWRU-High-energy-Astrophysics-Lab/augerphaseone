/*******************************************

  $Author: guglielmi $
  $Date: 2011-09-09 13:57:00 +0200 (Fri, 09 Sep 2011) $
  $Revision: 1511 $

********************************************/
/*
  Description


  Handle Queue Message
  **********

  History

  V1 - guglielm - 2009/07/09 Creation

*/
/**
 * @defgroup handle_msg  All Message Types Handling
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
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "can.h"

#include "msgsvr_version.h"
#include "logfile.h"
#include "shmlib.h"
#include "gpsstatus.h"
#include "gpsutil.h"
#include "msgqueuelib.h"
#include "msgsvr.h"
#include "svrconfig.h"
#include "central_local.h"
#include "msgsvrclient.h"
#include "candefs.h"
#include "canlib.h"
#include "handle_msg.h"
#include "linklib.h"
#include "tocdas.h"

extern int errno ;

/**************************************************
  Static Variables
***************************************************/

static MSG_CLIENT_STRUCT *FirstClient = NULL, *LastClient = NULL ;
#define PFIRST_CLIENT (void **)&FirstClient
#define PLAST_CLIENT  (void **)&LastClient
static int NbClients = 0 ;

/**************************************************
  Global Variables
***************************************************/

extern int Verbose ;

extern GPS_STATUS * GpsStatus ;
extern SVR_CONFIG SvrConfig ;
extern MSGSVR_STATUS * SvrStatus ;

/**************************************************
  Static Functions
***************************************************/
static char *Name ;
static int Nlength ;

static int compare_client( MSG_CLIENT_STRUCT *cur )
{
  return strncmp( cur->name, Name, Nlength ) ;
}

static void del_old_client( MSGSVR_CLIENT_CLOSE_MSG * msg )
{
  MSG_CLIENT_STRUCT * pcl ;

  if ( NbClients == 0 ) return ;
  // Get queue name
  Name = msg->header.name ;
  Nlength = strlen( Name ) ;
  if ( Verbose ) LogPrint( LOG_INFO, "Removing client '%s'\n", Name ) ;
  // Find the item in the linked list and delete
  if ( (pcl = LinkFind( FirstClient, LastClient, compare_client )) != NULL ) {
    LinkUnlink( pcl ) ;
    LinkDelete( pcl ) ;
    NbClients-- ;
    if ( Verbose )
      LogPrint( LOG_INFO, "Client removed, left: %d\n", NbClients ) ;
  }
  else if ( Verbose ) 
    LogPrint( LOG_WARNING, "Client '%s' NOT found !\n", Name ) ;

}

static void add_new_client( MSGSVR_CLIENT_OPEN_MSG * msg )
{
  MSG_CLIENT_STRUCT * pcl ;

  pcl = LinkCreate( sizeof( MSG_CLIENT_STRUCT ) ) ;
  strcpy( pcl->name, msg->header.name ) ;
  pcl->nb_id = msg->nb_id ;
  memcpy( pcl->msg_id, msg->msg_id, msg->nb_id ) ;
  /* Now take care of the queue */
  /* Open the queue */
  pcl->q_id = IpcMsgBind( pcl->name, &pcl->q_key ) ;

  if ( Verbose )
    LogPrint( LOG_INFO, "New client - '%s', Id: %u, Key: 0x%X\n",
	      pcl->name, pcl->q_id, pcl->q_key ) ;

  if ( LinkAddTail( pcl, PFIRST_CLIENT, PLAST_CLIENT ) != NULL ) {
    NbClients++ ;

    if ( Verbose ) {
      MSG_CLIENT_STRUCT * cur ;
      LogPrint( LOG_INFO, "    Added, Nb of Clients: %d\n", NbClients ) ;
      for( cur = FirstClient ; cur != NULL ; cur = LinkNext( cur ) )
	LogPrint( LOG_INFO, "    Client Name: '%s'\n", cur->name ) ;
    }
  }
  else
    LogPrint( LOG_ERROR, "    Could Not Add the client\n" ) ;
}

/**************************************************
  Global Functions
***************************************************/

/** 
 * This functions receives the messages from the clients.
 * Based on the message type it takes the appropiate action:
 *  If it is a msgsvr cmd, handle it immediately.
 *  If a message to the radio itself, handles it immediately.
 *  If it is a message for CDAS,  fills the frame as much as possible
 *   and transmit the frame to the Radio (CANBUS)
 * 
 * @param pkt Pointer to the message received from any client
 * 
 * @return 0 if OK, an error otherwise
 */
int HandleMessage( MSGSVR_PKT * pkt )
{
  int msg_size = pkt->header.length ;

  SvrStatus->from_client.nb_pkt++ ;
  SvrStatus->from_client.tot_size += msg_size ;
  if ( Verbose )
    LOG_DEBUG( LOG_INFO, " From Client msg size: %d, type: %d\n", msg_size,
	      pkt->header.type ) ;

  if ( pkt->header.type == MSGSVR_CMD ) {
    MSGSVR_CLIENT_HEADER_MSG * cur_msg = 
      (MSGSVR_CLIENT_HEADER_MSG *)pkt->payload ;
    time_t tt0 = time( NULL ) ;
    if ( Verbose )
      LOG_DEBUG( LOG_INFO,
		" Date: %u, Action: %d, Queue: %s\n", (unsigned int)tt0,
		cur_msg->header.action, cur_msg->header.name ) ;

    if ( cur_msg->header.action == MSGSVR_CLIENT_OPEN_ACTION )
      add_new_client( (MSGSVR_CLIENT_OPEN_MSG *)cur_msg ) ;
    else if ( cur_msg->header.action == MSGSVR_CLIENT_CLOSE_ACTION )
      del_old_client( (MSGSVR_CLIENT_CLOSE_MSG *)cur_msg ) ;
  }
  else if ( pkt->header.type == TO_RADIO_MSG ) {
    unsigned char * cur_msg = (unsigned char *)pkt->payload ;

    if ( Verbose )
      LOG_DEBUG( LOG_INFO, "Got a message to Radio: %d bytes\n",
		 msg_size );
    CanlibSendRadioMsg( cur_msg, msg_size ) ;
  }
  else {
    int nmsg ;

    /* Should not send to CDAS if wireless is not OK
       CAVEAT: the M-READY message should thus be (re)sent when wireless
         becomes OK ! Done elsewhere (in main msgsvr).
    */
    if ( SvrStatus->wireless != WIRELESS_STATUS_OK ) {
      if ( Verbose )
	LogPrint( LOG_WARNING,
		  "Wireless is BAD, Ignore message to CDAS\n" ) ;
      return -1 ;
    }
    /* Save the msg to CDAS */
    nmsg = ToCdasSave( pkt ) ;
    if ( nmsg != 0 && Verbose ) {
      LOG_DEBUG( LOG_INFO, " Nb msg Saved: %d (LsId: %d)\n", nmsg,
		SvrConfig.LsId ) ;
    }
  }

  return 0 ;
}

MSG_CLIENT_STRUCT * GetFirstClient()
{
  return FirstClient ;
}

int IsGoodClient( unsigned char id, MSG_CLIENT_STRUCT * pcl )
{
  int n ;

  if ( Verbose ) LOG_DEBUG( LOG_INFO, "Client '%s'\n", pcl->name ) ;
  for( n = 0 ; n<pcl->nb_id ; n++ ) {
    if ( Verbose )
      LogPrint( LOG_INFO, "pcl->id: %d, id: %d\n", pcl->msg_id[n], id ) ;
    if ( pcl->msg_id[n] == id ) {
      if ( Verbose )
	LogPrint( LOG_INFO, "Client %s found\n", pcl->name ) ;
      return 1 ;
    }
  }
  return 0 ;
}

/**@}*/

