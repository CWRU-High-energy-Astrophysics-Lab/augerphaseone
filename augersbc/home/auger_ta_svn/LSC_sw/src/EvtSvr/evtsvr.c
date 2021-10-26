/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-08-25 14:44:03 #$
  $Revision:: 1439             $

********************************************/

/*
  Description

  **********

  History

  V1 - guglielm - 2009/12/16 Creation

*/

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
#include <sys/wait.h>
#include <netinet/in.h>

/**
 * @defgroup event_svr Evtsvr: Event Server
 * @ingroup acquisition
 */

/**
 * @defgroup evtsvr  Main Module
 * @ingroup event_svr
 *
 * 
 */

/**@{*/
/**
 * @file   evtsvr.c
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Feb  9 14:40:38 2011
 * 
 * @brief  Event Server, sends Event data upon request from CDAS
 * 
 * 
 */

#include "logfile.h"
#include "memlib.h"
#include "shmlib.h"
#include "sigauger.h"
#include "buflib.h"
#include "acqstatus.h"
#include "gpsstatus.h"
#include "msgqueuelib.h"
#include "msgsvr.h"
#include "msgsvrclient.h"
#include "central_local.h"
#include "gpsutil.h"
#include "util.h"
#include "slowcontrol.h"
#include "ttaglib.h"
#include "acq_msg.h"
#include "events.h"
#include "calibx.h"
#include "calibh.h"
#include "t3_ttag.h"

#include "evtsvr_version.h"


/**************************************************
  Static Variables
***************************************************/

#define MY_PROG_NAME "evtsvr"
#define MY_LOG_FILE_NAME "evtsvr.log"

enum {VERSION_OPT} ;
static struct option long_options[] = {
  {"verbose", 0, 0, 'v'},
  {"version", 0, 0, VERSION_OPT},
  {"help", 0, 0, '?'},
  {0, 0, 0, 0}
};
static char *Options = "v?" ;

static char *Version ;
static char *Cvs = "$Author: guglielmi $ - $Revision: 1439 $ - $Date:: 2011-08-25 14:#$" ;

static int MyQueueId = -1 ;
static unsigned int MyQueueKey = 0 ;

static int SvrQueueId = -1 ;

acq_status_t * AcqStatus = NULL ;
static int AcqStatusId = -1 ;
static unsigned int AcqStatusKey = 0 ;

static GPS_STATUS * GpsStatus = NULL ;
static int GpsStatusId = -1 ;
static unsigned int GpsStatusKey = 0 ;

static BUFIdent EvtBuf = 0 ;
static int EvtBufId = -1 ;
static unsigned int EvtBufKey = 0 ;

#if EVTSVR_VERSION>5
BUFIdent CalibxBuf = 0 ; /* Used in xbtrig.c */
static int CalibxBufId = -1 ;
static unsigned int CalibxBufKey = 0 ;

BUFIdent CalibhBuf = 0 ; /* Used in xbtrig.c */
static int CalibhBufId = -1 ;
static unsigned int CalibhBufKey = 0 ;
#endif

#define MAX_ALREADY_KNOWN 5

/// Structure housing one saved event
typedef struct {
  short evtid ;
  char fname[128] ;
} ONE_SAVED_EVENT ;

static ONE_SAVED_EVENT AlreadyKnown[MAX_ALREADY_KNOWN] ;
static int NbAlready = 0 ;

static FAST_EVENT FoundEvent ;

static TO_CDAS_MESSAGE * ToCdas = NULL ;
static int ToCdasSize = 0 ;

/**************************************************
  Global Variables
***************************************************/

extern char *optarg ;
extern int optind ;

int Verbose = 0 ;

T3_YES_MESSAGE T3Msg ;

int DoCompress( void *buf, int size, short evid ) ;

/**************************************************
  Static Functions
***************************************************/

static void SendSomeEvent() ;

static void Help()
{
  fprintf( stderr, "%s\n", Version ) ;
  fputs( "Options:\n", stderr ) ;
  fputs( "     --version        : Print the version.", stderr ) ;
  fputs( " -v, --verbose        : Verbose\n", stderr ) ;
  fputs( " -?, --help           : What you see now\n", stderr ) ;

  exit( 1 ) ;
}

static void handleOptions( int argc, char **argv )
{
  int opt ;
  int option_index = 0;

  while (( opt = getopt_long (argc, argv, Options,
                              long_options, &option_index)) != EOF )
    switch( opt ) {
    case VERSION_OPT:
      fprintf( stderr, "%s\n", Version ) ;
      exit( 1 ) ;
    case 'v': Verbose++ ; break ;
    default: Help() ;
    }
}

static void setVersionCvs( char * prog )
{
  Version = mem_malloc( 128 ) ;

  sprintf( Version, "%s - V%d.%d\n%s", prog, EVTSVR_VERSION/10,
	   EVTSVR_VERSION % 10, Cvs ) ;

}

static void Finish()
{
  MsgSvrClientClose( MY_PROG_NAME, SvrQueueId ) ;
  if ( MyQueueKey != 0 ) IpcMsgRemove( MyQueueId ) ;
  if ( EvtBufId != -1 ) BuflibUnlink( EvtBuf, EvtBufId ) ;
}

static int Byebye()
{
  Finish() ;
  exit( 1 ) ;
}

static void gotSig( int sig )
{
  switch( sig ) {
  case SIGINT:
  case SIGQUIT:
    Byebye() ;
  default:
    // ignore
    break ;
  }
}

static void Initialize()
{
  /* Initialize Logfile */
  LogSetProgName( MY_PROG_NAME ) ;
  //LogSetFileName( MY_PROG_NAME ".log" ) ;
  LogSetNewFile( MY_LOG_FILE_NAME ) ;

  LogPrint( LOG_INFO, "Version '%s'\n", Version ) ;
  /*
    Initialize message server
  */
  /* Create my msg queue */
  MyQueueId = IpcMsgCreate( MY_PROG_NAME, &MyQueueKey ) ;
  LogPrint( LOG_INFO, "  Name '%s', Id: %d, Key: 0x%08X\n",
	    MY_PROG_NAME, MyQueueId, MyQueueKey) ;

  LogPrint( LOG_INFO, "Open MsgSvr connection:\n" ) ;

  unsigned char msg_id = M_T3_YES ;
  int nb_id = 1 ;

  SvrQueueId = MsgSvrClientOpen( MY_PROG_NAME, nb_id, &msg_id ) ;

  signal( SIGQUIT, gotSig ) ;
  signal( SIGINT, gotSig ) ;
  signal( SIGUSR1, gotSig ) ;

  /* Link to T1 Buffer Shared Memory */
  EvtBuf = BuflibLink( &EvtBufId, EVENT_BUFFER_NAME,
		   sizeof( FAST_EVENT ), EVENT_BUFFER_NB_EVENTS,
		   &EvtBufKey ) ;
  if ( EvtBuf == NULL ) {
    LogPrint( LOG_ERROR, "Cant Link T1 Events Cicrcular Buffer\n" ) ;
    Byebye( 1 ) ;
  }
  else LogPrint( LOG_INFO, "EvtBuf '%s' Key: 0x%08x\n",
		 EVENT_BUFFER_NAME, EvtBufKey ) ;

#if EVTSVR_VERSION>5
  /* Link to CalibX Buffer Shared Memory */
  CalibxBuf = BuflibLink( &CalibxBufId, CALIBX_BUFFER_NAME,
		   sizeof( CALIBX_BLOCK ), MAX_CALIBX_BUFFER,
		   &CalibxBufKey ) ;
  if ( CalibxBuf == NULL ) {
    LogPrintTimed( LOG_ERROR, "Cant Link CalibX Cicrcular Buffer\n" ) ;
    Byebye( 1 ) ;
  }
  else LogPrintTimed( LOG_INFO, "CalibxBuf '%s' Key: 0x%08x\n",
		 CALIBX_BUFFER_NAME, CalibxBufKey ) ;

  /* Link to CalibH Buffer Shared Memory */
  CalibhBuf = BuflibLink( &CalibhBufId, CALIBH_BUFFER_NAME,
		   sizeof( CALIBH_BLOCK ), MAX_CALIBH_BUFFER,
		   &CalibhBufKey ) ;
  if ( CalibhBuf == NULL ) {
    LogPrint( LOG_ERROR, "Cant Link CalibH Cicrcular Buffer\n" ) ;
    Byebye( 1 ) ;
  }
  else LogPrint( LOG_INFO, "CalibhBuf '%s' Key: 0x%08x\n",
		 CALIBH_BUFFER_NAME, CalibhBufKey ) ;
#endif

  /* Attacht acqstatus shared memory */
  AcqStatus = ShmAttach( ACQ_STATUS_NAME, sizeof( acq_status_t ),
			 &AcqStatusId, &AcqStatusKey ) ;
  if ( AcqStatus == NULL ) {
    LogPrint( LOG_WARNING, "Cant attach Acq Status\n" ) ;
    Byebye( 1 ) ;
  }
  else
    LogPrint( LOG_INFO, "AcqStatus Key = 0x%x\n", AcqStatusKey ) ;

  /* Attach GPS Status shared memory */
  GpsStatus = ShmAttach( GPS_STATUS_NAME, sizeof( GPS_STATUS ), &GpsStatusId,
			 &GpsStatusKey ) ;
  if ( GpsStatus == NULL )
    LogPrint( LOG_WARNING, "Cant attach GPS Status\n" ) ;
  else LogPrint( LOG_INFO, "GpsStatus Key: 0x%08x\n", GpsStatusKey ) ;

  memset( AlreadyKnown, 0, sizeof( AlreadyKnown ) ) ;

  TaskReady() ;

}

static void HandleSignal( MSGSVR_PKT * pkt )
{
  if ( Verbose )
    LogPrint( LOG_INFO, "Got a Signal Message\n" ) ;

  // get the signal value
  int sig ;

  sig = bytes_to_int( pkt->payload ) ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Got a signal msg: %d\n", sig ) ;
  switch( sig ) {
  case SIG_RESET_VERBOSE:
    Verbose = 0 ;
    break ;
  case SIG_INCREMENT_VERBOSE:
    Verbose++ ;
    break ;
  case SIG_DECREMENT_VERBOSE:
    if ( Verbose > 0 ) Verbose-- ;
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
  case SIG_EVTSVR_SEND_EVENT:
    /* Special, send the newest event available */
    SendSomeEvent() ;
    break ;
  default:
    gotSig( sig ) ;
    break ;
  }
}

/** 
 * Decode the ,message and fill the structure T3Msg. Containing the event ID,
 * date and time, offset, etc.
 * 
 * @param pkt The received packet
 * 
 * @return 0 if bad message, 1 otherwise
 */
static int DecodeMessage( MSGSVR_PKT * pkt )
{
  CDAS_MESSAGE_HEADER * msg ;
  short msg_len ;
  unsigned char type ;
  int position, nblsid ;

  msg = (CDAS_MESSAGE_HEADER *)pkt->payload ;
  msg_len = ntohs( msg->length ) ;
  type = msg->type ;

  if ( type != M_T3_YES ) {
    LogPrint( LOG_WARNING, "Unexpected Message Type: %d\n", type ) ;
    return 0 ;
  }

  /* Now get the message info */
  unsigned char * pt3 =  pkt->payload + MESSAGE_HEADER_LENGTH ;
  unsigned short evid ;

  evid = bytes_to_short( pt3 ) ; ;
  T3Msg.evtid = (evid & EVTID_MASK) ;
  T3Msg.again = (evid >> EVTID_AGAIN_SHIFT) & EVTID_AGAIN_MASK ;
  pt3 += sizeof( short ) ;
  T3Msg.time.second = bytes_to_int( pt3 ) ;
  pt3 += sizeof( int ) ;
  T3Msg.time.nano = bytes_to_int( pt3 ) ;
  pt3 += sizeof( int ) ;
  /* Get the microref and Delta according to position of the LS in the list.
     position and nb of LSs is passed in the Pkt Header
     NOTE that position starts at 1.
  */
  position = pkt->header.position ;
  nblsid =  pkt->header.nb_ls ;

  T3Msg.microref = *(pt3+position-1) ;
  pt3 += nblsid ;
  T3Msg.delta = *pt3++ ;

  LogPrint( LOG_INFO, "Evtid: %d (again = %d), Seconds: %d, Micros: %d\n",
	    T3Msg.evtid, T3Msg.again, T3Msg.time.second, T3Msg.time.nano );
  LogPrint( LOG_INFO, "LsId Position: %d, Nb of LsId: %d\n",
	    position, nblsid ) ;
  LogPrint( LOG_INFO, "MicroRef: %d, Delta: %d\n",  T3Msg.microref,
	    T3Msg.delta ) ;
  return 1 ;
}


/** 
 * Prepare the data to be sent to CDAS:
 *   Evtid (short )
 *   ErrorWord (short)
 *   The event (ONE_EVENT)
 *   Calibx
 *   Calibh
 *   TTag
 *
 *  Save to ram disk and compress
 *  Read back compressed file into output message
 * 
 * 
 * @return 0 or an error code.
 */
static int PrepareEvent()
{
  /*
    Add to output data:
    Evtid (short )
    ErrorWord (short)
    The event (ONE_EVENT)
    Calibx
    Calibh
    TTag

    Then save to ram disk and compress

    Finaly read back compressed file into output message

  */
  int tocdas_size, data_size = 0, new_data_size, header_size ;
  T3_EVT_MESSAGE_HEADER * t3evt ;
  FAST_EVENT * pevt ;
  unsigned char * pdata, * p0data ;

  header_size = sizeof( TO_CDAS_MESSAGE_HEADER ) +
    sizeof( T3_EVT_MESSAGE_HEADER ) ;
#if EVTSVR_VERSION<7
  tocdas_size = header_size + sizeof( FAST_EVENT ) + sizeof( TTAG_BLOCK ) ;
#else
  tocdas_size = header_size + sizeof( FAST_EVENT ) + sizeof( CALIBX_BLOCK ) +
    sizeof( CALIBH_BLOCK ) + sizeof( TTAG_BLOCK ) ;
#endif
  /*
    sizeof( CALIBX_BLOCK ) +
    sizeof( TTAG_BLOCK ) ;
  */

  if ( Verbose )
    LogPrint( LOG_INFO, "Msg Size: %d (%d + %d + %d)\n", tocdas_size,
	      header_size, sizeof( FAST_EVENT ), sizeof( TTAG_BLOCK )) ;
  ToCdas = mem_malloc( tocdas_size ) ;

  /* Now fill ToCdas */
  ToCdas->header.type = M_T3_EVT ;
  t3evt = (T3_EVT_MESSAGE_HEADER *)ToCdas->payload ;

  t3evt->evtid = htons( T3Msg.evtid ) ;
  t3evt->compressed = 1 ;
  t3evt->error = 0 ;

  pevt = (FAST_EVENT *)(ToCdas->payload + sizeof( TO_CDAS_MESSAGE_HEADER )) ;
  pdata = (unsigned char *)pevt ;
  p0data = pdata ;

  if ( Verbose )
    LogPrint( LOG_INFO, "   Ready to add event at %p, data size: %d\n",
	      pdata, data_size ) ;

  /* Now fill the event part */
  memcpy( pevt, &FoundEvent, sizeof( FAST_EVENT ) ) ;
  data_size += sizeof( FAST_EVENT ) ;
  pdata += sizeof( FAST_EVENT ) ;
  LogPrint( LOG_INFO, "   Event added to msg, next ptr: %p, data size: %d\n",
	    pdata, data_size ) ;

  /* Add Calibx */
#if EVTSVR_VERSION<6
  /* No calibx yet ! */
  memset( pdata, 0, sizeof( int ) ) ;
  pdata += sizeof( int ) ;
  data_size += sizeof( int ) ;
#else
  CALIBX_BLOCK calx ;
  int the_size = sizeof( CALIBX_BLOCK) ;


  /* Get newest calibx block and copy to message */
  int ok ;

  ok = BuflibGetNew( CalibxBuf, &calx ) ;
  if ( ok == BUF_REQUEST_NOT_FOUND ) {
    /* No calibx yet ! */
    memset( pdata, 0, sizeof( int ) ) ;
    pdata += sizeof( int ) ;
    data_size += sizeof( int ) ;
  }
  else {
    /* set the size of the calibx block in the buffer */
    memcpy( pdata, &the_size, sizeof( int ) ) ;
    pdata += sizeof( int ) ;
    data_size += sizeof( int ) ;
    memcpy( pdata, &calx, sizeof( CALIBX_BLOCK ) ) ;
    data_size += sizeof( CALIBX_BLOCK ) ;
    pdata += sizeof( CALIBX_BLOCK ) ;
  }
#endif

  /* Add CalibH */
#if EVTSVR_VERSION<6
  /* No histograms yet */
  memset( pdata, 0, sizeof( int ) ) ;
  pdata += sizeof( int ) ;
  data_size += sizeof( int ) ;
#else
  CALIBH_BLOCK calh ;


  ok = BuflibGetNew( CalibhBuf, &calh ) ;
  if ( ok == BUF_REQUEST_NOT_FOUND ) {
    /* No histograms yet */
    memset( pdata, 0, sizeof( int ) ) ;
    pdata += sizeof( int ) ;
    data_size += sizeof( int ) ;
  }
  else {
    the_size = sizeof( CALIBH_BLOCK) ;
    memcpy( pdata, &the_size, sizeof( int ) ) ;
    pdata += sizeof( int ) ;
    data_size += sizeof( int ) ;
    memcpy( pdata, &calh, sizeof( CALIBH_BLOCK ) ) ;
    data_size += sizeof( CALIBH_BLOCK ) ;
    pdata += sizeof( CALIBH_BLOCK ) ;
  }
  if ( Verbose )
    LogPrint( LOG_INFO,
	      "   CalibH added to msg, next ptr: %p, data size: %d\n",
	      pdata, data_size ) ;
#endif

  /*
    Add Ttag data
    Use memcpy to avoid any potential alignement problem
  */
  int idxpp,			/**< Index of second -2 in sawtooth table */
    idxp,			/**< Index of second -1 ... */
    idxc,			/**< Index of current second ... */
    idxn ;			/**< Index of second+1 ... */

  idxpp = (FoundEvent.date.first.second-2) & SAWTOOTH_MASK ;
  idxp  = (FoundEvent.date.first.second-1) & SAWTOOTH_MASK ;
  idxc  = (FoundEvent.date.first.second) & SAWTOOTH_MASK ;
  idxn  = (FoundEvent.date.first.second+1) & SAWTOOTH_MASK ;

  TTAG_BLOCK the_ttag ;

  if ( Verbose )
    LogPrint( LOG_INFO, "   Prepare TTAG Block at %p\n", pdata ) ;
  /* Add calib100 (second and second+1) */
  the_ttag.cur_100 = GpsStatus->sawtooth.table[idxc].calib100;
  the_ttag.next_100 = GpsStatus->sawtooth.table[idxn].calib100 ;

  /* Add sawtooth (second-2, second-1 and second )
     Sawtooth are actually (signed char), but passed as unsigned int in the
     ttag block
 */
  the_ttag.pprev_saw = GpsStatus->sawtooth.table[idxpp].sawtooth ;
  the_ttag.prev_saw = GpsStatus->sawtooth.table[idxp].sawtooth ;
  the_ttag.cur_saw = GpsStatus->sawtooth.table[idxc].sawtooth ;

  /* Add Gps Offset (From gpsconfig, copied into gpsstatus at startup) */
  the_ttag.rcv_offset = GpsStatus->receiver_offset ;

  memcpy( pdata, &the_ttag, sizeof( TTAG_BLOCK ) ) ;
  pdata += sizeof( TTAG_BLOCK ) ;
  data_size += sizeof( TTAG_BLOCK ) ;
  if ( Verbose )
    LogPrint( LOG_INFO, "   TTAG added to msg, next ptr: %p, data size: %d\n",
	      pdata, data_size ) ;

  /* Save to file, Compress and read back */
  new_data_size = DoCompress( p0data, data_size, T3Msg.evtid ) ;
  ToCdasSize = new_data_size + header_size ;

  return NO_ERROR ;
}

int Compare( const void * pkey, const void * ptr )
{
  FAST_EVENT * pevt = (FAST_EVENT *)ptr ;
  BUF_SEARCH_KEY * key = (BUF_SEARCH_KEY *)pkey ;
  double cursec ;

  cursec = pevt->date.first.second +
    (double)(pevt->date.first.nano + pevt->micro_off*100)/100000000. ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Compare data: %lf vs Key: %lf, %lf\n",
	      cursec, key->sec_min, key->sec_max ) ;
  if ( cursec > key->sec_max ) return -1 ;
  else if ( cursec < key->sec_min ) return 1 ;
  else return 0 ; 
}

/** 
 * 
 * Search an event in the event buffer. Return NO_ERROR if found,
 * one of M_T3_NOT_FOUND, M_T3_TOO_OLD otherwise
 * 
 * @return An error code or NO_ERROR
 */
static int SearchEvent()
{
  int tagidx = 0 ;
  u_char oldtag = 0, newtag = 1 ;
  int err, ret ;
  BUF_SEARCH_KEY the_key ;

  BuflibSetKey( T3Msg.time.second, T3Msg.microref, T3Msg.time.nano,
		T3Msg.delta, &the_key ) ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Search in [%.6lf, %.6lf]\n",
	      the_key.sec_min, the_key.sec_max ) ;
  int eidx ;			/**< @brief Index of found event (if any) */
  err = BuflibFindTag( EvtBuf, (void *)&FoundEvent, Compare, &the_key,
		       &eidx, tagidx, oldtag, newtag ) ;
  switch ( err ) {
  case BUF_OK:
    ret = NO_ERROR ;
    break ;
  case BUF_REQUEST_NOT_FOUND:
    ret = M_T3_NOT_FOUND ;
    break ;
  case BUF_REQUEST_ALREADY:
    ret = M_T3_ALREADY ;
    break ;
  case BUF_REQUEST_TOO_OLD:
    ret = M_T3_LOST ;
    break ;
  case BUF_REQUEST_TOO_YOUNG:
    ret = M_T3_TOO_YOUNG ;
    break ;
  default:
    ret = M_T3_UNKNOWN ;
    break ;
  }
  if ( err != NO_ERROR )
    LogPrint( LOG_INFO, "SearchEvent ret = %d. err: %d - %s\n", ret, err,
	      BuflibErrorStr( err ) ) ;
  else
    LogPrint( LOG_INFO, "SearchEvent Found: %d.%08d\n",
	      FoundEvent.date.first.second, FoundEvent.date.first.nano ) ;

  return ret ;
}

static void AddAlready( int evid )
{
  int i, max ;

  if ( NbAlready < MAX_ALREADY_KNOWN ) {
    i = NbAlready ;
  }
  else {
    /* Move AlreadyKnown table (make place for new one) */
    unlink( AlreadyKnown[0].fname ) ;
    memmove( &AlreadyKnown[0], &AlreadyKnown[1], sizeof(ONE_SAVED_EVENT)*4 ) ;
    i = MAX_ALREADY_KNOWN - 1 ;
  }
  AlreadyKnown[i].evtid = evid ;
  sprintf( AlreadyKnown[i].fname, "t3_%05d.bz2", evid ) ;
  NbAlready++ ;
  max = i ;

  if ( Verbose ) {
    LogPrint( LOG_INFO, "  Nb Already: %d\n", NbAlready ) ;
    for( i = 0 ; i <= max  ; i++ ) {
      LogPrint( LOG_INFO, " Already %d, '%s'\n", i, AlreadyKnown[i].fname ) ;
    }
  }

}

static int IsSameId()
{
  int i ;

  for( i=0 ; i<MAX_ALREADY_KNOWN ; i++ )
    if ( AlreadyKnown[i].evtid == T3Msg.evtid ) return i ;

  return -1 ;
}

static int EventFound()
{
  int ok = NO_ERROR ;
  /*

  if (again && IsSameId() ) {
  prepare event ;
  return NO_ERROR ;
  }
  else if ( IsSameId() ) {
  ignore request ;
  return M_T3_ALREADY ;
  }
  else if ( (ok = Search_event()) == 0 ) {
  prepare event ;
  return NO_ERROR ;
  }
  else return ok ;
  */
  ok = IsSameId() ;
  if ( T3Msg.again && ok != -1 ) {
    /* the event was already sent, resend it */
    PrepareEvent() ;
    return NO_ERROR ;
  }
  else if ( T3Msg.again == 0 && ok != -1 ) {
    /* the event was already sent, dont send again */
    return M_T3_ALREADY ;
  }
  else if ( (ok = SearchEvent()) == NO_ERROR ) {
    PrepareEvent() ;
    AddAlready( T3Msg.evtid ) ;
    return NO_ERROR ;
  }
  else return ok ;

}

/** 
 * Send the newest event available upon request by signal SIG_EVTSVR_SEND_EVENT
 * 
 */
static void SendSomeEvent()
{
  /* Get newest event */
  int err = 0 ;

  err = BuflibGetNew( EvtBuf, (void *)&FoundEvent ) ;
  if ( err == NO_ERROR ) {
    /* Prepare it and send */
    T3Msg.evtid = 0 ;
    PrepareEvent() ;
    MsgSvrClientSend( SvrQueueId, LOW_PRIORITY, ToCdas, ToCdasSize ) ;
    LogPrint( LOG_INFO, "Sent T3 on signal - type = %d, size = %d\n",
		     ToCdas->header.type, ToCdasSize ) ;
    mem_free( ToCdas ) ;
  }
}

static void SendEvent()
{
  /* Just for the fun, returns an error (T3_NOT_FOUND) */
  T3_EVT_MESSAGE_HEADER * t3evt ;
  int tocdas_size ;
  int ok ;

  ok = EventFound() ;

  if ( ok == NO_ERROR ) {
    /* Send the event. TO BE WRITTEN */
    MsgSvrClientSend( SvrQueueId, LOW_PRIORITY, ToCdas, ToCdasSize ) ;
    LogPrint( LOG_INFO, "Sent T3 - type = %d, size = %d\n",
		     ToCdas->header.type, ToCdasSize ) ;
    mem_free( ToCdas ) ;
  }
  else {
    tocdas_size = sizeof( T3_EVT_MESSAGE_HEADER ) +
      sizeof( TO_CDAS_MESSAGE_HEADER ) ;
    ToCdas = mem_malloc( tocdas_size ) ;
    ToCdas->header.type = M_T3_EVT ;

    t3evt = (T3_EVT_MESSAGE_HEADER *)ToCdas->payload ;

    t3evt->evtid = htons( T3Msg.evtid ) ;
    t3evt->compressed = 1 ;
    t3evt->error = ok ;

    MsgSvrClientSend( SvrQueueId, LOW_PRIORITY, ToCdas, tocdas_size ) ;
    LogPrint( LOG_INFO, "Sent T3 - type = %d, size = %d\n",
		     ToCdas->header.type, tocdas_size ) ;
    mem_free( ToCdas ) ;
  }
}

/**************************************************
  Global Functions
***************************************************/


int main( int argc, char **argv )
{
  MSGSVR_PKT the_pkt ;

  setVersionCvs( "evtsvr" ) ;

  handleOptions( argc, argv ) ;

  Initialize() ;

  /* Infinite Loop */
  for( ; ; ) {
    /* Wait for a message from msgsvr */
    int size ;

    size = msgrcv( MyQueueId, &the_pkt, MAX_MSGSVR_PAYLOAD,
		   0, IPC_NOWAIT ) ;

    /* Handle the message */
    if ( size<= 0 ) {
      usleep( 100000 ) ;
      continue ;
    }
    /* Check if a signal */
    if ( the_pkt.header.type == SIGNAL_MSG ) {
      HandleSignal( &the_pkt ) ;
      continue ;
    }
    /* Now M_T3_Yes */
    if ( DecodeMessage( &the_pkt ) == 0 ) continue ; /* Something wrong, not
							the good type */
    /* Now search the event and send something */
    SendEvent() ;
  }

  return 0 ;
}

/**@}*/
