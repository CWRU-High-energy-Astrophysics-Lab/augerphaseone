/*******************************************

  $Author: guglielmi $
  $Date: 2011-11-22 12:41:04 +0100 (Tue, 22 Nov 2011) $
  $Revision: 1776 $

********************************************/


/**
 * @defgroup services Services Processes
 *
 */
/**
 * @defgroup msgsvr  Msgsvr: Message Server
 * @ingroup services
 *
 * The message server handles the connections with he radio and the
 * messages to/from CDAS. Any process wishing to communicate with CDAS must
 * register itself to the msgsvr
, indicating which (if any) message types it
 * expects to receive from CDAS. NOTE that more than one task may receive the
 * same messages from CDAS.
 * 
 * Read frames from canbus and distributes to the relevant process.
 * Get messages from other processes and send to radio.
 * Messages are received from msg queues, one queue per priority, 3 priorities:
 *   High, Medium, Low

 *
 */
/**@{*/

#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#define __USE_BSD
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <arpa/inet.h>

#include "svr_version.h"
#include "can.h"

#include "msgsvr_version.h"
#include "logfile.h"
#include "cpunumber.h"
#include "shmlib.h"
#include "gpsstatus.h"
#include "gpsutil.h"
#include "svrconfig.h"
#include "msgqueuelib.h"
#include "msgsvr.h"
#include "msgsvrclient.h"
#include "handle_msg.h"
#include "central_local.h"
#include "candefs.h"
#include "canlib.h"
#include "handle_radio.h"
#include "handle_can_msg.h"
#include "ppsclientlib.h"
#include "sigauger.h"
#include "memlib.h"
#include "tocdas.h"
#include "acklist.h"
#include "util.h"
#include "lrstatus.h"

static struct option long_options[] = {
  {"boot", required_argument, 0, 'b'},
  {"logfile", required_argument, 0, 'L'},
  {"verbose", 0, 0, 'v'},
  {"help", 0, 0, '?'},
  {0, 0, 0, 0}
};
static const char * Options = "L:v?" ;

static char *Version ;
static const char *Cvs = "$Author: guglielmi $ - $Revision: 1776 $ - $Date: 2011-11-22 12:41:04 +0100 (Tue, 22 Nov 2011) $" ;

static int IsBoot = 0 ;

int Verbose = 0 ;

FILE *LogFile = NULL ;
char *LogPath = NULL ;

LR_STATUS * LrStatus = NULL ;
static int LrStatusId = -1 ;
static unsigned int LrStatusKey = 0 ;

GPS_STATUS * GpsStatus = NULL ;
static int GpsStatusId = -1 ;
static unsigned int GpsStatusKey = 0 ;

static int GotPPS = 0 ;

int PpsIrqReady = 0 ;

SVR_CONFIG SvrConfig ;

MSGSVR_STATUS * SvrStatus = NULL ;
static int SvrStatusId = -1 ;
static unsigned int SvrStatusKey = 0 ;

static int SvrQueueId = 0 ;
static unsigned int SvrQueueKey ;

static int Sig = 0 ;

static int CanFd = -1 ;

static int WirelessForcedOk = 0 ;

void HandleErrorMsg( void ) ;
void Byebye( int err ) __attribute__((noreturn)) ;
static void gotInt( int sig ) __attribute__((noreturn)) ;
static void gotQuit( int sig ) __attribute__((noreturn)) ;

static void Help( void )
{
  fprintf( LogFile, "%s\n", Version ) ;
  fputs( "Options:\n", LogFile ) ;
  fputs( " -L, --logfile=<path> : Logfile\n", LogFile ) ;
  fputs( " -v, --verbose        : Verbose\n", LogFile ) ;
  fputs( " -?, --help           : What you see now\n", LogFile ) ;

  exit( 1 ) ;
}

static void handleOptions( int argc, char **argv )
{
  int opt ;
  int option_index = 0;

  while (( opt = getopt_long (argc, argv, Options,
                              long_options, &option_index)) != EOF )
    switch( opt ) {
    case 'b':
      sscanf( optarg, "%d", &IsBoot ) ;
      break ;
    case 'L': LogPath = mem_malloc( strlen( optarg ) + 1 ) ;
      strcpy( LogPath, optarg ) ;
      break ;
    case 'v': Verbose++ ; break ;
    default: Help() ;
    }
}

static void setVersionCvs( const char * prog )
{
  Version = mem_malloc( 128 ) ;

  sprintf( Version, "%s - V%d.%d\n%s", prog, MSGSVR_VERSION/10,
	   MSGSVR_VERSION % 10, Cvs ) ;

}

void Byebye( int err )
{
  /* Stop Das in case */
  LogPrint( LOG_INFO, "Stopping Das (in case it is still running)\n" ) ;

  system( "das -vv stop >>/ram0/das.log &" ) ;

  /* Wait till das is stopped */
  int wcount = 0 ;
  while ( is_running( "das" ) > 0 ) {
    if ( wcount > 10 ) {
      /* Kill previous das stop
	 Start "das -v kill"
      */
      system( "stop -9 das" ) ;
      sleep( 5 ) ;
      system( "das -v kill >>das.log" ) ;
      wcount = 0 ;
    }
    else {
      LogPrint( LOG_INFO, "Das still running, wait\n" ) ;
      sleep( 1 ) ;
      wcount++ ;
    }
  }
  LogPrint( LOG_INFO, "Das Stopped after %d seconds\n", wcount ) ;

  /* Tell radio that we are stopping */
  POWER_STATUS_SEND_MSG rd_msg ;
  rd_msg.source = SOURCE_DEST_LS ;
  rd_msg.valid = 0 ;
  SendToRadio( POWER_STATUS_SEND, &rd_msg, POWER_STATUS_SEND_MSG_LENGTH ) ;

  if ( SvrQueueId > 0 ) IpcMsgRemove( SvrQueueId ) ;
  if ( GpsStatus != NULL ) ShmRemove( GpsStatus, GpsStatusId ) ;
  if ( SvrStatus != NULL ) ShmRemove( SvrStatus, SvrStatusId ) ;
  if ( LrStatus != NULL ) ShmRemove( LrStatus, LrStatusId ) ;
  LogPrint( LOG_FATAL, "Killed by %d\n", err ) ;

  exit( err ) ;
}

static void gotQuit( int sig )
{
  Byebye( sig ) ;
}

static void gotInt( int sig )
{
  Byebye( sig ) ;
}

static void gotUsr( int sig )
{
  Sig = sig ;
}

static void init_config()
{
  // Initialise svr config
  FILE * fin ;

  SvrConfig.CpuNumber = GetCpuNumber() ;

  if ( (fin = fopen( SVR_CONFIG_FILE_NAME, "r" )) == NULL ) {
    //SvrConfig.CpuNumber = GetCpuNumber() ;
    SvrConfig.LsId = SvrConfig.CpuNumber ;
  }
  else {
    int dummy ;
    fread( &dummy, sizeof( int ), 1, fin ) ;
    fread( &SvrConfig.LsId, sizeof( int ), 1, fin ) ;
    fread( SvrConfig.LsName, sizeof( SvrConfig.LsName ), 1, fin ) ;
    fclose( fin ) ;
  }
  SvrConfig.initialized = 0xBABEFACE ;
  SvrConfig.ServicesVersion = SVR_VERSION ;
  SvrStatus->ServicesVersion = SVR_VERSION ;
  SvrStatus->CpuNumber = SvrConfig.CpuNumber ;

  LogPrint( LOG_INFO, "CpuNUmber: %d, LsId: %d\n", SvrConfig.CpuNumber,
	    SvrConfig.LsId ) ;
  LogPrint( LOG_INFO, "LsName '%s'\n", SvrConfig.LsName ) ;
  LogPrint( LOG_INFO, "Services Version: %08X\n", SvrConfig.ServicesVersion ) ;
}

/** 
 * Initialisation. Load SvrConfig file, Attach to GpsStatus shared memory,
 * Create the message queue, Open the Can Socket, Register to PpsIrq.
 * 
 */
static void Initialize( void )
{
  // Init logfile library
  if ( LogPath == NULL ) {
    LogPath = mem_malloc( strlen( MSGSVR_QUEUE_NAME ) + 8 ) ;
    sprintf( LogPath, "%s.log", MSGSVR_QUEUE_NAME ) ;
  }
  LogSetNewFile( LogPath ) ;
  LogSetProgName( MSGSVR_QUEUE_NAME ) ;
  LogPrint( LOG_INFO, "%s\n", Version ) ;
  LogPrint( LOG_INFO, "Started after Boot: %d\n", IsBoot ) ;

  SvrStatus = ShmAttach( (char *)MSGSVR_STATUS_NAME,
			 sizeof( MSGSVR_STATUS ), &SvrStatusId,
			 &SvrStatusKey ) ;
  if ( SvrStatus == NULL ) {
    LogPrint( LOG_FATAL, "Cant attach Msgsvr Status Shared Memory\n" ) ;
    exit( 1 ) ;
  }
  LogPrint( LOG_INFO, "SvrStatus Key: 0x%08x\n", SvrStatusKey ) ;
  memset( SvrStatus, 0, sizeof( MSGSVR_STATUS ) ) ;

  // Initialize svr config
  init_config() ;

  // Attach to gps status shared mem
  GpsStatus = ShmAttach( (char *)GPS_STATUS_NAME, sizeof( GPS_STATUS ),
			 &GpsStatusId, &GpsStatusKey ) ;
  if ( GpsStatus == NULL ) {
    LogPrint( LOG_FATAL, "Cant attach Gps Status Shared Memory\n" ) ;
    exit( 1 ) ;
  }
  LogPrint( LOG_INFO, "GpsStatus Key: 0x%08x\n", GpsStatusKey ) ;

  LrStatus = ShmAttach( (char *)LR_STATUS_NAME, sizeof( LR_STATUS ),
			&LrStatusId, &LrStatusKey ) ;
  if ( LrStatus == NULL ) {
    LogPrint( LOG_FATAL, "Cant attach LR Status Shared Memory\n" ) ;
    exit( 1 ) ;
  }
  memset( LrStatus, 0, sizeof( LR_STATUS ) ) ;
  LrStatus->Initialized = SHARED_MEM_INITIALIZED ;

  LogPrint( LOG_INFO, "LR Status Key: 0x%08x\n", LrStatusKey ) ;

  // Create my message queue
  LogPrint( LOG_INFO, "Msg Queue: '%s'\n", MSGSVR_QUEUE_NAME ) ;
  SvrQueueId = IpcMsgCreate( MSGSVR_QUEUE_NAME, &SvrQueueKey ) ;
  LogPrint( LOG_INFO, "Msg Queue Id: %d, Key: 0x%08X\n", SvrQueueId,
		   SvrQueueKey ) ;

  if ( GpsStatus->initialized != SHARED_MEM_INITIALIZED ) {
    LogPrint( LOG_WARNING, "GPS Status Shared Mem NOT Initialized\n" ) ;
    LogPrint( LOG_WARNING, "Come back later\n" ) ;
  }

  // Prepare signals
  signal( SIGQUIT, gotQuit ) ;
  signal( SIGINT, gotInt ) ;
  signal( SIGUSR1, gotUsr ) ;
  signal( SIGUSR2, gotUsr ) ;

  // Open Can socket
  CanFd = CanlibOpen() ;
  if ( CanFd < 0 ) {
    LogPrint( LOG_ERROR, "Can't open Can Device '%s'\n",
	      CAN_DEVICE ) ;
    Byebye( errno ) ;
  }
  else {
    LogPrint( LOG_INFO, "Can Device '%s' Opened: %d\n",
	      CAN_DEVICE, CanFd ) ;
    /*
      Now filter radio only. TBD
    */
  }


  if ( PpsClientRegister( MSGSVR_QUEUE_NAME ) == 0 )
    PpsIrqReady = 1 ;

  /* First message to Radio */
  /* Send a message to radio "just rebooted" */
  POWER_STATUS_SEND_MSG rd_msg ;
  rd_msg.source = SOURCE_DEST_LS ;
  rd_msg.valid = 0xFF ;
  SendToRadio( POWER_STATUS_SEND, &rd_msg, POWER_STATUS_SEND_MSG_LENGTH ) ;

  TaskReady() ;

  LogPrint( LOG_INFO, " Initialized\n" ) ;

}

static int get_acq_status()
{
  int status = 7 ;

  system( "acqstatus" ) ;
  FILE *fin ;
  fin = fopen( "/ram0/exit_code", "r" ) ;
  fscanf( fin, "%d", &status ) ;
  fclose( fin ) ;
  return status ;
}

/** 
 * Build and send an M_READY Message to CDAS.
 * The wireless status can be
 *   - MREADY_WIRELESS_WAS_OK: sent upon request from CDAS (M_WAKEUP)
 *   - MREADY_WIRELESS_WAS_BAD: sent when wireless was BAD, is now OK.
 *
 * @param str A comment written to log file
 * @param wstat Wireless status. 
 */
static void send_mready( char * str, int wstat )
{
  TO_CDAS_MESSAGE * tocdas ;
  M_READY_MESSAGE * msg ;
  int tocdas_len ;
  char status ;
  char pwr ;
  int gps_status ;

  /* Get Acq Status */
  status = get_acq_status() ;

  /* get poweron/reset */
  pwr = isPowerOn() ;

  tocdas_len = sizeof( M_READY_MESSAGE ) + sizeof( TO_CDAS_MESSAGE_HEADER ) ;
  tocdas = mem_malloc( tocdas_len ) ;
  msg = (M_READY_MESSAGE *)tocdas->payload ;

  if ( Verbose )
    LogPrint( LOG_INFO, "M_Ready - Header length: %d, msg length: %d [0x%x]\n",
	      sizeof( TO_CDAS_MESSAGE_HEADER ), sizeof( M_READY_MESSAGE ),
	      sizeof( M_READY_MESSAGE ) ) ;

  tocdas->header.type = M_READY ;

  // Build M_READY Message
  if ( GpsStatus->traim_status.traim_solution == TRAIM_SOLUTION_OK)
    gps_status = 0 ;
  else gps_status = 1 ;
  msg->run_status = status ;
  msg->poweron_reset = pwr | wstat | (IsBoot<<1) | (gps_status << 3) ;
  msg->software_version = htonl( SvrStatus->ServicesVersion ) ;
  msg->config_version = htonl( 1 ) ;
  msg->northing = htonl( GpsStatus->position_utm.northing ) ;
  msg->easting = htonl( GpsStatus->position_utm.easting ) ;
  msg->height = htonl( GpsStatus->position_mas.height ) ;
  //  strcpy( msg->utm.zone, GpsStatus->position_utm.zone ) ;
  msg->cur_time = htonl( GpsStatus->gps_current_time ) ;

  /* Version 7: alignment pb */
  short utc_off = GpsStatus->gps_utc_offset ;
  msg->utc_offset = htons( utc_off ) ;
  /* Version 6
     msg->utc_offset = htonl( GpsStatus->gps_utc_offset ) ;
  */

  // Send to msgsvr
  MsgSvrClientSend( SvrQueueId, HIGH_PRIORITY, tocdas, tocdas_len ) ;
  LogPrint( LOG_INFO, "%s - Sent M_READY, Gps Time: %u\n", str,
		   GpsStatus->gps_current_time ) ;

  mem_free( tocdas ) ;
  return ;
}

void HandleErrorMsg( void )
{
  if ( errno == EIDRM ) {
    LogPrint( LOG_FATAL, "Message queue Removed: %s\n",
	       strerror( errno ) ) ;
    SvrQueueId = 0 ;
    Byebye( errno ) ;
  }
  else if ( errno == ENOMSG ) {
    //printf( "No Message\n" ) ;
    return ;
  }
  else printf( "Unexpected error %d, %s\n", errno, strerror( errno ) ) ;
}

void HandleSignal( MSGSVR_PKT * msg )
{
  int sig ;

  sig = bytes_to_int( msg->payload ) ;
  switch( sig ) {
  case SIG_RESET_VERBOSE: Verbose = 0 ;
    break ;
  case SIG_INCREMENT_VERBOSE: Verbose++ ;
    break ;
  case SIG_DECREMENT_VERBOSE: Verbose-- ;
    break ;
  case SIG_MEMLIB_STATUS_LOG:
    {
      MEMLIB_STATUS m_status ;
      mem_status( &m_status ) ;
      LogPrint( LOG_INFO, " Total Allocated Memory: 0x%x (%d) - %d Blocks\n",
		m_status.tot_mem, m_status.tot_mem, m_status.nb_blocks ) ;
      LogPrint( LOG_INFO, " Maximum Allocated Memory: 0x%x (%d)\n",
		m_status.tot_mem, m_status.tot_mem ) ;	
    }
    break ;
  case SIGUSR1:
  case SIGUSR2:
    gotUsr( sig ) ;
    break ;
  case SIGQUIT:
    gotQuit( sig ) ;
    break ;
  case SIGINT:
    gotInt( sig ) ;
    break ;
  case PPS_SIGNAL_READY:
    GotPPS = 1 ;
    if ( Verbose > 1 ) 
      LogPrint( LOG_INFO, "Got 1 PPS\n" ) ;
    break ;
  case SIG_READ_CONFIG:
    init_config() ;
    break ;
  case SIG_GPS_HAS_DATE:
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
  case SIG_PPS_IS_GOOD:
  case SIG_PPS_IS_BAD:
    {
      /* Signal from gpsctrl in case of PPS coming BAD/GOOD
      */
      if ( Verbose )
	LogPrint( LOG_INFO, "Got Signal %d from Gpsctrl\n", sig ) ;
      PPS_STATUS_REPLY_MSG msg ;

      if ( sig == SIG_PPS_IS_GOOD ) msg.valid = 0xff ;
      else msg.valid = 0 ;
      SendToRadio( GPS_1PPS_STATUS, &msg, 1 ) ;
    }
    break ;
  case SIG_GPS_READY:
    /* Signal from Gpsctrl when GPS receiver is ready
       Send the M_READY msg to CDAS
    */
    if ( Verbose )
      LogPrint( LOG_INFO, "Got Signal %d (GPS Ready) from Gpsctrl\n", sig ) ;
    send_mready( "Got SIG_GPS_READY", MREADY_WIRELESS_WAS_OK ) ;
    break ;
  case SIG_SAVE_CONFIG:
    LogPrint( LOG_INFO, "Save configuration files to USB key\n" ) ;
    system( "saveconfig &" ) ;
    break ;
  case SIG_FORCE_WIRELESS_OK:
    /* Force the wireless status to OK */
    WirelessForcedOk = 1 ;
    SvrStatus->wireless = WIRELESS_STATUS_OK ;
    break ;
  case SIG_FORCE_WIRELESS_BAD:
    /* Force the wireless status to OK */
    WirelessForcedOk = 0 ;
    SvrStatus->wireless = WIRELESS_STATUS_BAD ;
    break ;
  default:
    /* Ignore */
    LogPrint( LOG_WARNING, "Got Spurious signal %d\n", sig ) ;
    break ;
  }
}

#define REQ_WNET_MAX 5
/** 
 * Every REQ_WNET_MAX seconds, request the wireless status if it was BAD.
 *
 * 
 * @param flag 0 to reset the count, 1 to increment
 */
static void req_wireless( int flag )
{
  static int count = 0 ;

  if ( flag == 0 ) {
    count = 0 ;
    return ;
  }

  count++ ;
  if ( count >= REQ_WNET_MAX ) {
    unsigned char msg = SOURCE_DEST_LS ;
    SendToRadio( WIRELESS_NET_REQ, &msg, 1 ) ;
    count = 0 ;
  }
}

/** 
 * Check if the wireless status has changed. If yes, 2 possibilities:
 *  - the status was OK and is now bad. In this case, stop sending
 *    to CDAS, clear the pending messages if any, do not allow more messages.
 *  - the status was bad and is back OK. In this case send M_READY and allow
 *    messages to CDAS.
 * .
 *
 * @param[in] wstat Current status of the wireless
 *
 */
void wireless_change( int wstat )
{
  int status ;

  if ( wstat == RADIO_WIRELESS_NORMAL || WirelessForcedOk == 1 )
    status = WIRELESS_STATUS_OK ;
  else status = WIRELESS_STATUS_BAD ;

  if ( SvrStatus->wireless == status ) return ;
  if ( status != WIRELESS_STATUS_OK ) {
    /* Lost wireless */
    /* Reset saved messages */
    LogPrint( LOG_WARNING, "Wireless BAD\n" ) ;
  }
  else {
    /* Wireless back ok */
    /* Send M_READY again with pwron-reset + Radio back */
    send_mready( "Wireless back OK", MREADY_WIRELESS_WAS_BAD ) ;
    req_wireless( 0 ) ;
  }
  SvrStatus->wireless = status ;
}


int main( int argc, char **argv )
{
  MSGSVR_PKT pkt ;
  struct timeval tmout ;
  fd_set rdfs ;
  int nbfs, retval ;

  LogFile = stdout ;

  setVersionCvs( "msgsvr" ) ;

  handleOptions( argc, argv ) ;

  Initialize() ;

  nbfs = CanFd + 1 ;

  // Ad vitam aeternam loop
  for( ; ; ) {
    int size ;

    // Wait for ever. Awakened by input from CAN OR message in the queue
    // We mix select and msgrcv:
    // Select 10 millis then if nothing, msgrcv no-wait
    FD_ZERO( &rdfs ) ;
    tmout.tv_sec = 0 ;
    tmout.tv_usec = 10000 ; /* 10 millis */
    FD_SET( CanFd, &rdfs ) ;

    retval = select( nbfs, &rdfs, NULL, NULL, &tmout ) ;
    if ( retval > 0 ) {
      if ( FD_ISSET( CanFd, &rdfs ) ) {
	int ll ;
	canmsg_t canframe ;

	ll = CanlibGetDataMsg( &canframe ) ;
	FD_CLR( CanFd, &rdfs ) ;
	if ( ll > 0 ) {
	  if ( Verbose )
	    LogPrint( LOG_INFO, "Got from CAN, Id: %x, Length: %d\n",
		      canframe.id, ll ) ;
	  HandleCanFrame( &canframe ) ;
	}
	else LogPrint( LOG_WARNING, "Received 0 bytes from CAN Socket\n" ) ;
      }
    }
    else {
      if ( errno == EINTR ) {
	//LogPrint( LOG_INFO, "Awakened from 'select' by signal %d\n", Sig ) ;
	Sig = 0 ;
	errno = 0 ;
      }
      else {
	// Check if any incoming messages from queue
	size = msgrcv( SvrQueueId, &pkt, MAX_MSGSVR_PAYLOAD,
		       0, IPC_NOWAIT ) ;
	if ( size > 0 ) {
	  if ( Verbose > 1 )
	    LogPrint( LOG_INFO, "Got Message type: %d, Size: %d (LsId = %d\n",
		      pkt.header.type, size, SvrConfig.LsId ) ;
	  if ( pkt.header.type == SIGNAL_MSG ) {
	    HandleSignal( &pkt ) ;
	    if ( GotPPS == 1 ) {
	      /* Got 1PPS, Send messages to CDAS if Wireless is OK */
	      int nmsg = 0 ;
	      GotPPS = 0 ;
	      if ( SvrStatus->wireless != WIRELESS_STATUS_OK ) {
		/* Request Wireless Status every N seconds*/
		req_wireless( 1 ) ;
	      }
	      /* Check that there is NO pending ACK.
		 In principle, we should receive the ACK within the same
		 second when we sent the data stream, thus when the 1PPS
		 arrives, AnyAck should be 0.
		 If it's not the case ... what ?
	      */
	      if ( AnyAck() != 0 ) {
		LogPrint( LOG_WARNING, "Ack Pending !!!\n" ) ;
		if ( SvrStatus->wireless != WIRELESS_STATUS_OK ) {
		  /* clear pending Ack */
		  RemAck() ;
		}
		else {
		  /* Need to do something special ?
		     Wait till the next 1PPS */
		}
	      }
	      else if ( (nmsg = ToCdasLeft()) != 0 ) {
		/* Now check waiting messages and send */
		if ( Verbose )
		  LogPrint( LOG_INFO,
				   "Saved Messages to send: %d\n", nmsg ) ;
		ToCdasSend() ;
	      }
	    }
	  }
	  else HandleMessage( &pkt ) ;
	}
	else if ( errno != EINTR ) {
	  Sig = 0 ;
	  HandleErrorMsg() ;
	  errno = 0 ;
	}
	else if ( errno == EINTR ) {
	  LogPrint( LOG_INFO, "Awakened from 'msgrcv' by signal %d\n", Sig ) ;
	  Sig = 0 ;
	  errno = 0 ;
	}
      }
    }
    if ( !PpsIrqReady && PpsClientRegister( MSGSVR_QUEUE_NAME ) == 0 ) {
      /* PPS IRQ running ==> GPS receiver OK, should tell Radio:
       Send GPS Status
       Send Time
       Send Position
       What else ?
       Request Wireless Status
       Launch DAS !
      */
      LogInitTime( &GpsStatus->gps_current_time ) ;
      PpsIrqReady = 1 ;
      system( "das -v start >/ram0/das.log &" ) ;
      LogPrint( LOG_INFO, "1PPS OK, Das Launched\n" ) ;
    }
  }

  Byebye( 0 ) ;
  return 0 ;
}

/**@}*/
