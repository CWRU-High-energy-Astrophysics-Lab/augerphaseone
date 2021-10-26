/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-09-29 15:23:25 #$
  $Revision:: 1581             $

********************************************/

/*
  Description

  This process runs on the PC104/Arm9 "grey box". It simulates the radio.
  Connected to the LSC via CAN Bus and to Concentrator via TCP_IP.
  NB: The concentrator itself is connected to Pm via TCP-IP.

  **********

  History

  V1 - guglielm - 2010/05/25 Creation
  V10 - guglielm - 2015/05/27 Adapted to socketcan (not tested yet !)

*/

/**
 * @defgroup csradio  MTU Radio Emulator
 * @ingroup services
 *
 * @brief Emulates the behaviour of the MTU radio.
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
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <linux/can.h>
#include <linux/if.h>


#include "csradio_version.h"

#include "candefs.h"
#include "tpcb.h"
#include "canlib.h"
#include "logfile.h"
#include "gpsutil.h"
#include "msgqueuelib.h"


/**************************************************
  Static Variables
***************************************************/

#define MY_PROG_NAME "csradio"
#define MY_LOG_FILE_NAME "csradio.log"
#define CAN_MSG_LENGTH 8

#if !defined(AF_CAN)
// dont know where this AF_CAN is !!
#define AF_CAN 29
#define PF_CAN AF_CAN
#endif

#if !defined(EINTR)
#define EINTR 4
#define EAGAIN 11
#endif

enum { PORT_OPT, CAN_OPT, VERSION_OPT} ;
static struct option long_options[] = {
  {"port", required_argument, 0, PORT_OPT},
  {"can", required_argument, 0, CAN_OPT},
  {"verbose", 0, 0, 'v'},
  {"version", 0, 0, VERSION_OPT},
  {"help", 0, 0, '?'},
  {0, 0, 0, 0}
};
static char *Options = "v?" ;

static char *Version ;
static char *Cvs = "$Author: guglielmi $ - $Revision: 1581 $ - $Date:: 2011-09-29 15:#$" ;

static int CanSocket = -1, CanConnectedSocket = -1 ;
static int CanConnected = 0 ;
static struct sockaddr_in FromCan, FromCanConnected ;
static int FromCanLen, FromCanConnectedLen ;
static char CanDev[8] ;

static short CsPort = 20900 ;
static int CsSocket = -1, CsConnectedSocket = -1 ;
static int CsConnected = 0 ;
static struct sockaddr_in FromCs, FromCsConnected ;
static int FromCsLen, FromCsConnectedLen ;

static unsigned char * CdasMsg = NULL ;
static int CdasMsgLength = 0 ;

static int LscReady = 0 ;

static int CrQueueId = 0 ;
static unsigned int CrQueueKey ;

typedef struct {
  long type ;
  int length ;
  char msg[256] ;
} ONE_MSG ;

static int WirelessStatus = 0 ; /* Bad by default, good when the CS is connected */


#define AUTO_TPCB_TIME 60
static time_t NextTpcb = 0 ;
static int TpcbSim = 0 ;
static struct can_frame Frame100, Frame101 ;

/**************************************************
  Global Variables
***************************************************/

extern char *optarg ;
extern int optind ;

int Verbose = 0 ;
int MsgReceived = 0, MsgSent = 0, OvrErr = 0 ;
time_t LastCanSent ;

void * SvrStatus = NULL ;

/**************************************************
  Static Functions
***************************************************/
static int send_wireless_status( unsigned char status ) ;

static void Help()
{
  printf( "%s\n", Version ) ;
  puts( "Options:" ) ;
  puts( "     --port=<nnnn>    : Port number (default 43210)" ) ;
  puts( "     --can=<device>   : Can Device name. Default 'can0'" ) ;
  puts( "     --version        : Print the version." ) ;
  puts( " -v, --verbose        : Verbose" ) ;
  puts( " -?, --help           : What you see now" ) ;

  exit( 1 ) ;
}

static void handleOptions( int argc, char **argv )
{
  int opt ;
  int option_index = 0;

  while (( opt = getopt_long (argc, argv, Options,
                              long_options, &option_index)) != EOF )
    switch( opt ) {
    case PORT_OPT:
      {
        int pnb ;

        sscanf( optarg, "%d", &pnb ) ;
        CsPort = pnb ;
      }
      break ;
    case CAN_OPT:
      strcpy( CanDev, optarg ) ;
      break ;
    case VERSION_OPT:
      printf( "%s\n", Version ) ;
      exit( 1 ) ;
    case 'v': Verbose++ ; break ;
    default: Help() ;
    }
}

static void setVersionCvs( char * prog )
{
  Version = malloc( 128 ) ;

  sprintf( Version, "%s - V%d.%d\n%s", prog, CSRADIO_VERSION/10,
	   CSRADIO_VERSION % 10, Cvs ) ;

}

static void Finish()
{
  send_wireless_status( RADIO_WIRELESS_LISTEN ) ;
  if ( CanSocket > 0 ) CanlibClose() ;
  if ( CsSocket != -1 ) close( CsSocket ) ;
  //if ( CrQueueId > 0 ) IpcMsgRemove( CrQueueId ) ;

}

static int Byebye( void )
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
  case SIGUSR1:
    Verbose++ ;
    printf( "Verbose = %d\n", Verbose ) ; fflush( stdout ) ;
    break ;
  case SIGUSR2:
    Verbose = 0 ;
    printf( "Verbose = %d\n", Verbose ) ; fflush( stdout ) ;
    break ;
  default:
    // ignore
    break ;
  }
}

/**
 * Open the socket to Concentrator Station.
 *
 */
static int OpenCsSocket()
{
  int err = 0 ;
  struct        hostent         hostentstruct;
  struct        hostent         *hostentptr;
  char hostme[32] ; 

  CsSocket = socket( AF_INET, SOCK_STREAM, 0);
  LogPrint( LOG_INFO, "CsSocket: %d\n", CsSocket ) ;

  if ( CsSocket < 0) {
    return -1 ;
  }
  gethostname( hostme, sizeof( hostme ) ) ;
  LogPrint( LOG_INFO, "Host Name: %s\n", hostme ) ;
#if 0
  hostentptr = gethostbyname( hostme ) ;

  hostentstruct = *hostentptr ;
#endif
  
  FromCsLen = sizeof( FromCs );
  bzero( &FromCs, FromCsLen ) ;
  FromCs.sin_family=AF_INET;
  FromCs.sin_port = htons( CsPort ) ;
  int yes = 1 ;
  err = setsockopt( CsSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&yes,
	      sizeof( socklen_t ) ) ;
  LogPrint( LOG_INFO, "Setsockopt: %d\n", err ) ;
  if ( err != 0 ) {
    return err ;
    
  }
  
  err = 0 ;
  while (bind( CsSocket, (struct sockaddr *)&FromCs, FromCsLen ) < 0) {
    if ( err == 0 ) {
      printf( "Bind error\n" ) ; fflush( stdout ) ;
      LogPrint( LOG_WARNING, "Bind error\n" ) ;
    }
    err = -1 ;
    sleep( 1 ) ;
  }

  LogPrint( LOG_INFO, "Socket bound - CdasPort: %d\n",
	    htons( FromCs.sin_port) ) ;

  err = listen( CsSocket, 1 ) ;


  return err ;
}

static int OpenCanSocket( void )
{
  int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct can_frame frame;
  int err = 0 ;
  
  if ((CanSocket = socket(family, type, proto)) < 0) {
    perror("CAN socket");
    err = -1 ;
  }
  else {
    addr.can_family = family;
    strcpy(ifr.ifr_name, CanDev);
    ioctl(CanSocket, SIOCGIFINDEX, &ifr);
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(CanSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("bind");
      err = -1 ;
    }
  }
  /* Now initialize CanLib */
  CanlibOpen( CanSocket ) ;
  return err ;
}

static void Initialize()
{
  time_t ttt = time( NULL ) ;

  /* Initialize Logfile */
  LogSetProgName( MY_PROG_NAME ) ;
  //LogSetFileName( MY_PROG_NAME ".log" ) ;
  LogSetNewFile( MY_LOG_FILE_NAME ) ;

  printf( "Pid: %d\n", getpid() ) ;
  fflush( stdout ) ;

  LogPrint( LOG_INFO, "Initializing - %s", ctime( &ttt ) ) ;
  LogPrint( LOG_INFO, "Version '%s'\n", Version ) ;

  /* Create CsSocket */
  if ( OpenCsSocket() != 0 ) {
    printf( "Can't create CS socket\n" ) ;
    exit( 1 ) ;
  }
  else {
    printf( "CS socket created: %d\n", CsSocket ) ; fflush( stdout ) ;
    LogPrint( LOG_INFO, "CsSocket Created : %d\n", CsSocket ) ;
  }

  // Create CAN socket
  if ( OpenCanSocket() != 0 ) {
    printf( "Can't create CAN socket\n" ) ; fflush( stdout ) ;
    exit( 1 ) ;
  }
  else {
    printf( "CAN socket created: %d\n", CanSocket ) ;
    fflush( stdout ) ;
    LogPrint( LOG_INFO, "CanSocket Created : %d\n", CanSocket ) ;
  }
  

  // Create my message queue
  LogPrint( LOG_INFO, "Msg Queue: '%s'\n", MY_PROG_NAME ) ;
  //CrQueueId = IpcMsgCreate( MY_PROG_NAME, &CrQueueKey ) ;
  //LogPrint( LOG_INFO, "Msg Queue Id: %d, Key: 0x%08X\n", CrQueueId, CrQueueKey ) ;


  signal( SIGQUIT, gotSig ) ;
  signal( SIGINT, gotSig ) ;
  signal( SIGUSR1, gotSig ) ;
  signal( SIGUSR2, gotSig ) ;

  LogPrint( LOG_INFO, "Initialized\n" ) ;
}

static int dumpFrame( struct can_frame * frame, int len )
{
  char str[128] ;
  int i ;

  sprintf( str, "<<< 0x%03x:", (unsigned int)frame->can_id ) ;

  for ( i = 0 ; i<len ; i++ ) {
    char s[8] ;
    sprintf( s, " %02X", frame->data[i] ) ;
    strcat( str, s ) ;
  }
  if ( Verbose ) {
#if 0
    printf( "%s - %d.%06d\n", str, (int)frame->timestamp.tv_sec,
	    (int)frame->timestamp.tv_usec ) ;
    fflush( stdout ) ;
#endif
    if ( frame->can_id >= DATA_COMMAND_DOWN_BEGIN &&
	 frame->can_id <= DATA_COMMAND_DOWN_MAX ) {
#if 0
      LogPrint( LOG_INFO,
		"%s - Down - %d.%06d\n", str, frame->timestamp.tv_sec,
		frame->timestamp.tv_usec ) ;
#endif
    }
#if 0
    else {
      LogPrint( LOG_INFO, "%s - %d.%06d\n", str, frame->timestamp.tv_sec,
		frame->timestamp.tv_usec ) ;
    }
#endif
  }
  return 1 ;
}

static void DumpCsMsg( unsigned char * buf, int n )
{
  char * str ;
  char s[8] ;
  int i ;

  LogPrint( LOG_INFO, "Dump msg Pkt\n" ) ;

  str = malloc( 256 ) ;
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
}

static int GetCsMsg()
{
  short len ;
  int n ;
  unsigned char * buf ;

  n = read( CsConnectedSocket, &len, sizeof( len ) ) ;
  len = ntohs( len ) ;
  if ( n != sizeof( len ) ) {
    if ( Verbose ) printf( "Got %d/%d bytes from Cs, something wrong\n",
			   n, (int)sizeof( len ) ) ;
    if ( Verbose ) 
      LogPrint( LOG_WARNING, "Got %d/%d bytes from Cs, something wrong\n",
		 n, sizeof( len ) ) ;
    return -1 ;
  }
  else if ( Verbose > 1 ) {
    printf( "Got msg from CS, length: %d\n", len ) ;
    LogPrint( LOG_INFO, "Got msg length: %d\n", len ) ;
  }
  CdasMsgLength = len ;
  CdasMsg = malloc( CdasMsgLength + 2 ) ;
  buf = CdasMsg ;
  short_to_bytes( buf, (short)CdasMsgLength ) ;

  n = read( CsConnectedSocket, buf + 2, CdasMsgLength ) ;
  if ( n != CdasMsgLength ) {
    if ( Verbose ) printf( "Got %d/%d bytes from Cdas, something wrong\n",
			   n, CdasMsgLength ) ;
    if ( Verbose )
      LogPrint( LOG_WARNING, "Got %d/%d bytes from Cdas, something wrong\n",
		 n, CdasMsgLength ) ;
    free( CdasMsg ) ;
    return -1 ;
  }
  else {
    CdasMsgLength += 2 ;
    if ( Verbose > 1 ) {
      printf( "Got msg: %d bytes (Total: %d)\n",
	      n, CdasMsgLength ) ;
      LogPrint( LOG_INFO, "Got msg: %d bytes (Total: %d)\n",
		 n, len + 2 ) ;
      DumpCsMsg( CdasMsg, CdasMsgLength ) ;
    }
    /* Send msg to CAN */

    return 1 ;
  }
}

int SendToCan()
{
  /* Split CdasMsg into Can frames and write to canfd */
  int nb ;
  struct can_frame frame ;
  int can_id ;
  int can_begin, can_max, can_end ;
  unsigned char * pmsg ;
  int err ;
  
  if ( Verbose > 1 ) printf( "SendToCan, %d bytes\n", CdasMsgLength ) ;

  can_begin = DATA_COMMAND_DOWN_BEGIN ;
  can_max = DATA_COMMAND_DOWN_MAX ;
  can_end = DATA_COMMAND_DOWN_END ;
  can_id = can_begin ;

  for( nb = 0, pmsg = CdasMsg ; nb < CdasMsgLength ; nb += 8 ) {
    struct can_frame frame ;
    int i, j, last ;
    frame.can_id = can_id ;
    last = nb + 8 ;
    if ( last >= CdasMsgLength ) last = CdasMsgLength ;
    memset( frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;

    for ( j = 0, i = nb ; i < last ; i++, j++ ) {
      frame.data[j] = *pmsg++ ;
    }
    //CanWrite( CanFd, &frame, 1 ) ;
    err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
    if ( err != 0 ) {
      LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
		 strerror( errno) ) ;
      printf( "Can't send CAN frame ! %s\n", strerror( errno ) ) ;
      return( err );
    }
    MsgSent++ ;
    can_id += 2 ;
    if ( can_id >can_max ) can_id = can_begin ;
  }
  frame.can_id = can_end ;
  memset( frame.data, CAN_PADDING_BYTE, CAN_MSG_LENGTH ) ;
  frame.data[0] = CdasMsgLength  & 0xff ;
  frame.data[1] = ( CdasMsgLength >> 8 ) & 0xff ;
  //CanWrite( CanFd, &frame, 1 ) ;
  err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
  if ( err != 0 ) {
    LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
	       strerror( errno) ) ;
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }
  MsgSent++ ;

  /* Now free CdasMsg */
  free( CdasMsg ) ;
  return 0 ;
}

static void RadioFrame( struct can_frame * frame, int len )
{
  time_t ttt = time( NULL ) ;

  if ( frame->can_id < 0x100 ) {
    if ( Verbose ) {
      printf( "%u - LSC to Radio: 0x%03x, %d\n",
	      (unsigned int)ttt, (unsigned short)frame->can_id,
	      len ) ;
      int i ;
      for( i = 0 ; i<len ; i++ ) printf( " %02x", frame->data[i] ) ;
      printf( "\n" ) ;
      fflush( stdout ) ;
    }
  }

  /* Set LSC Ready as soon as we receive this kind of frame */
  LscReady = 1 ;

  switch (frame->can_id ) {
  case DOWN_STREAM_ACK:
    if ( Verbose ) {
      printf( "%u - Got Ack DOWN: %d bytes, Error: %x\n", (unsigned int)ttt,
	      bytes_to_short( &frame->data[2] ), frame->data[4] ) ;
      fflush( stdout ) ;
      LogPrint( LOG_INFO, "Got Ack DOWN: %d bytes, Error: %x\n",
		bytes_to_short( &frame->data[2] ), frame->data[4] ) ;
    }
    break ;
  case UP_STREAM_ACK:
    if ( Verbose ) {
      printf( "%u - Got Ack UP: Frame: %02x, %d bytes, Error: %x\n",
	      (unsigned int)ttt,
	      bytes_to_short( &frame->data[0] ),
	      bytes_to_short( &frame->data[2] ),frame->data[4] ) ;
    }
    break ;
  case POWER_STATUS_SEND:
    if ( frame->data[1] == 0xFF ) {
      if ( Verbose )
	printf( "%u - LSC Ready\n", (unsigned int)ttt ) ;
      LscReady = 1 ;
      fflush( stdout ) ;
    }
    else {
      if ( Verbose ) printf( "%u - LSC NOT Ready\n", (unsigned int)ttt ) ;
      LscReady = 0 ;
      fflush( stdout ) ;
    }
    break ;
  case WIRELESS_NET_REQ:
    /* LSC requests Wireless status
       Answer with WIRELESS STATUS (0xFF = OK, other = bad)
    */
    if ( WirelessStatus == 1 ) 
      send_wireless_status( RADIO_WIRELESS_NORMAL ) ;
    else send_wireless_status( RADIO_WIRELESS_LISTEN ) ;
    break ;
  }

  return ;
}

int sendUpAck( int nbytes )
{
  struct can_frame frame ;
  short sn = nbytes ;
  int err ;
  
  frame.can_id = UP_STREAM_ACK ;
  memset( frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
  short_to_bytes( frame.data, htons( DATA_COMMAND_UP_END ) ) ;
  short_to_bytes( &frame.data[2], htons( sn ) ) ;
  frame.data[4] = 0xFF ;
  err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
  if ( err != 0 ) {
    LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
	       strerror( errno) ) ;
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }

  return 0 ;
}

unsigned char ToCsMsg[1024], * CurToCsMsg = ToCsMsg ;
int ToCsMsgLength = 0 ;

/*
  Concatenate UP frames into a single array up to the END_UP frame
  Discard any other frame ID (for the moment)
  Return 0 if an UP frame, 1 if the message is complete
*/
int BuildMessage( struct can_frame * frame, int len )
{
  if ( Verbose > 1 ) printf( ">>>> BuildMessage: %d\n", len ) ;
  if ( frame->can_id < DATA_COMMAND_UP_BEGIN ||
       frame->can_id > DATA_COMMAND_UP_END ) {
    RadioFrame( frame, len ) ;
    return 0 ;
  }
  else if ( frame->can_id == DATA_COMMAND_UP_END ) {
    ToCsMsgLength = htons( bytes_to_short( frame->data ) ) ;
    sendUpAck( ToCsMsgLength ) ;
    return 1 ;
  }
  else {
    memcpy( CurToCsMsg, frame->data, len ) ;
    CurToCsMsg += len ;
    return 0 ;
  }
}

void SendToCs()
{
  /* Write to Cs Socket */
  int ln ;

  if ( Verbose > 1 ) printf( "SendToCs: %d\n", ToCsMsgLength ) ;
  if ( CsConnected ) {
    if ( Verbose ) DumpCsMsg( ToCsMsg, ToCsMsgLength ) ;
    ln = write( CsConnectedSocket, ToCsMsg, ToCsMsgLength ) ;
    if ( ln != ToCsMsgLength ) {
      if ( Verbose ) printf( "Write Error %s\n", strerror( errno ) ) ;
      return ;
    }
  }
  else if ( Verbose ) {
    printf( "Concentrator NOT connected\n" ) ;
    LogPrint( LOG_WARNING, "Concentrator NOT connected\n" ) ;
  }
  /* free ToCsMsg */
  CurToCsMsg = ToCsMsg ;
  ToCsMsgLength = 0 ;
}

static int req_echo()
{
  ECHO_MSG msg ;
  struct can_frame frame ;
  int err ;

  frame.can_id = ECHO_REQ ;
  memset( frame.data, 0xAA, CAN_PAYLOAD_LENGTH ) ;
  frame.data[0] = SOURCE_DEST_LS ;
  err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
  if ( err != 0 ) {
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }
  return 0 ;
}


static int req_pps_status()
{
  struct can_frame frame ;
  int err ;
  
  frame.can_id = GPS_1PPS_STATUS_REQ ;
  memset( frame.data, 0x55, CAN_PAYLOAD_LENGTH ) ;
  frame.data[0] = SOURCE_DEST_LS ;
  err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
  if ( err != 0 ) {
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }
  return 0 ;
}

static int req_gps_time()
{
  struct can_frame frame ;
  int err ;

  frame.can_id = GPS_DATE_TIME_REQ ;
  memset( frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
  frame.data[0] = SOURCE_DEST_LS ;
  err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
  if ( err != 0 ) {
    LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
	       strerror( errno) ) ;
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }
  return 0 ;
}

static int send_wireless_status( unsigned char status )
{
  struct can_frame frame ;
  int err ;

  frame.can_id = WIRELESS_NET_STATUS ;
  memset( frame.data, 0x55, CAN_PAYLOAD_LENGTH ) ;
  frame.data[0] = status ;
  err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
  if ( err != 0 ) {
    LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
	       strerror( errno) ) ;
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }
  return 0 ;
}

static int send_tpcb()
{
  int err ;
  
  if ( Verbose )
    LogPrint( LOG_INFO, "Sending TPCB Frames\n" ) ;
  err = CanlibWriteFrame( &Frame100, CAN_PAYLOAD_LENGTH, 0 ) ;
  if ( err != 0 ) {
    LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
	       strerror( errno) ) ;
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }
  err = CanlibWriteFrame( &Frame101, CAN_PAYLOAD_LENGTH, 0 ) ;
  if ( err != 0 ) {
    LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
	       strerror( errno) ) ;
    printf( "Can't send CAN frame !\n" ) ;
    return( err );
  }
  return 0 ;
}

static void auto_tpcb()
{
  if ( LscReady == 0 ) return ;

  time_t now = time( NULL ) ;

  if ( TpcbSim != 0 && now >= NextTpcb ) {
    if ( Verbose ) {
      printf( "*** Auto TPCB at %u\n", (unsigned int)now ) ;
      fflush( stdout ) ;
    }
    send_tpcb() ;
    NextTpcb = now + AUTO_TPCB_TIME ;
  }
}

#define AUTO_REQ_TIME 5
static time_t NextReq = 0 ;

static void auto_req()
{
  if ( LscReady == 0 ) return ;

  time_t now = time( NULL ) ;

  if ( NextReq != 0 && now >= NextReq ) {
    if ( Verbose )
      printf( "*** Auto Req at %u\n", (unsigned int)now ) ; fflush( stdout ) ;
    req_echo() ;
    req_pps_status() ;
    req_gps_time() ;
    NextReq = now + AUTO_REQ_TIME ;
  }

}

#define AUTO_MONIT_TIME 64
static time_t NextMonit = 0 ;
#define MONIT_COUNT_MAX 100
static int MonitCount = 0 ;

static int send_monit()
{
  struct can_frame frame ;
  int type ;
  int max, i ;
  int err ;
  
  /* Send CGD monit data (voltages and Temperatures) */
  frame.can_id = ROUTINE_MONITOR_DATA_CGD ;
  frame.data[7] = SOURCE_DEST_LR_CGD ;

  for( i = 0 ; i<CGD_MONITORING_NUMBER ; i++, MonitCount++ ) {
    frame.data[0] = i ;
    frame.data[1] = MonitCount ;
    frame.data[2] = MonitCount+1 ;
    frame.data[3] = MonitCount+2 ;
    if ( i == 0 ) frame.data[4] = MonitCount+3 ;
    else frame.data[4] = (293 - 164.3)/1.044 ; // temp 20 C
    frame.data[5] = MonitCount+4 ;
    frame.data[6] = MonitCount+5 ;
    /* Now send the frame */
    err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
    if ( err != 0 ) {
      LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
		 strerror( errno) ) ;
      printf( "Can't send CAN frame !\n" ) ;
      return( err );
    }
  }

  /* Send LR monit data (LR and RF performances) */
  frame.can_id = ROUTINE_MONITOR_DATA_LR ;
  frame.data[7] = SOURCE_DEST_LR_CPU | SOURCE_DEST_LR_IOP ;

  for( i = 0 ; i<LR_MONITORING_NUMBER ; i++, MonitCount++ ) {
    frame.data[0] = i ;
    /* Special for Last bad CAB ID */
    if ( i == 0 ) {
      /* CAN Id = MonitCount */
      frame.data[1] = MonitCount ;
      frame.data[2] = 0 ;
    }
    else {
      frame.data[1] = MonitCount ;
      frame.data[2] = MonitCount+1 ;
    }
    frame.data[3] = MonitCount+2 ;
    frame.data[4] = MonitCount+3 ;
    frame.data[5] = MonitCount+4 ;
    frame.data[6] = MonitCount+5 ;
    /* Now send the frame */
    err = CanlibWriteFrame( &frame, CAN_PAYLOAD_LENGTH, 1 ) ;
    if ( err != 0 ) {
      LogPrint( LOG_WARNING, "Can't send CAN frame ! Error %d, %s\n", errno,
		 strerror( errno) ) ;
      printf( "Can't send CAN frame !\n" ) ;
      return( err );
    }
  }

  if ( MonitCount > MONIT_COUNT_MAX ) MonitCount = 0 ;
  return 0 ;
}

static void auto_monit()
{
  if ( LscReady == 0 ) return ;

  time_t now = time( NULL ) ;

  if ( NextMonit != 0 && now >= NextMonit ) {
    if ( Verbose ) {
      printf( "*** Auto Monit at %u\n", (unsigned int)now ) ;
      fflush( stdout ) ;
    }
    send_monit() ;
    NextMonit = now + AUTO_MONIT_TIME ;
  }
}

static void get_msg()
{
  ONE_MSG the_msg ;
  int size ;

  size = msgrcv( CrQueueId, &the_msg, sizeof( ONE_MSG ), 0, IPC_NOWAIT ) ;
  if ( size < 0 ) return ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Got message - length = %d '%s'\n", the_msg.length,
	      the_msg.msg ) ;
  if ( strcmp( the_msg.msg, "woff" ) == 0 ) {
    if ( Verbose )LogPrint( LOG_INFO, "Set Wireless OFF\n" ) ;
    send_wireless_status( RADIO_WIRELESS_LISTEN ) ;
    WirelessStatus = 0 ;
  }
  else if ( strcmp( the_msg.msg, "won" ) == 0 ) {
    if ( Verbose ) LogPrint( LOG_INFO, "Set Wireless ON\n" ) ;
    send_wireless_status( RADIO_WIRELESS_NORMAL ) ;
    WirelessStatus = 1 ;
  }
  else if ( strcmp( the_msg.msg, "ton" ) == 0 ) {
    TpcbSim = 1 ;
    LogPrint( LOG_INFO, "Set TpcbSim ON\n" ) ;
  }
  else if ( strcmp( the_msg.msg, "toff" ) == 0 ) {
    TpcbSim = 0 ;
    LogPrint( LOG_INFO, "Set TpcbSim OFF\n" ) ;
  }
  else if ( strcmp( the_msg.msg, "+v" ) == 0 ) Verbose++ ;
  else if ( strcmp( the_msg.msg, "-v" ) == 0 ) {
    Verbose-- ;
    if ( Verbose < 0 ) Verbose = 0 ;
  }
  else if ( strcmp( the_msg.msg, "quit" ) == 0 ) {
    Byebye() ;
  }
  else if ( Verbose )LogPrint( LOG_INFO, "Ignored\n" ) ;
}

/**************************************************
  Global Functions
***************************************************/


int main( int argc, char **argv )
{
  int ret ;
  fd_set rdfs ;
  struct timeval tmout ;
  struct can_frame frame ;
  int max_fd ;

  setVersionCvs( "csradio" ) ;

  strcpy( CanDev, "can0" ) ;
  
  handleOptions( argc, argv ) ;

  Initialize() ;

  max_fd = CanSocket + 1 ;
  
  printf( "Waiting Cs Connection\n" ) ;
  fflush( stdout ) ;
  LogPrint( LOG_INFO, "Waiting Cs Connection\n" ) ;

  NextReq = time( NULL ) + AUTO_REQ_TIME ;
  NextMonit = time( NULL ) + AUTO_MONIT_TIME ;

  for( ; ; ) {
    int cs_sock ;

    if ( CsConnectedSocket == -1 ) {
      max_fd = CsSocket + 1 ;
      cs_sock = CsSocket ;
    }
    else {
      max_fd = CsConnectedSocket + 1 ;
      cs_sock = CsConnectedSocket ;
    }
    FD_ZERO( &rdfs ) ;
    FD_SET( CanSocket, &rdfs ) ;
    FD_SET( cs_sock, &rdfs ) ;
    FD_SET( 0, &rdfs ) ;

    tmout.tv_sec = 0 ;
    tmout.tv_usec = 100000 ;

    ret = select( max_fd, &rdfs, NULL, NULL, &tmout ) ;
    if ( ret < 0  ) {
      if ( errno != EINTR ) {
	LogPrint( LOG_FATAL, "Error on Select: %s\n", strerror( errno ) ) ;
	Byebye() ;
      }
      else {
	printf( "\nFrames IN; %d, OUT: %d, OVR: %d\n",
		MsgReceived, MsgSent, OvrErr ) ;
	if ( CsConnectedSocket == -1 )
	  printf( "Cs NOT connected\n" ) ;
	else
	  printf( "Cs CONNECTED\n" ) ;
	fflush( stdout ) ;
      }
      continue ;
    }
    /* Every 5 seconds, send ECHO, 1PPS status, GPS time request */
    auto_req() ;
    /* Every 64 seconds, send routine monitor data */
    auto_monit() ;
      
    get_msg() ;

    if ( FD_ISSET( cs_sock, &rdfs ) ) {
      if ( CsConnectedSocket == -1 ) {
	CsConnectedSocket = accept( CsSocket,
				    (struct sockaddr *)&FromCsConnected,
				    (socklen_t *)&FromCsConnectedLen ) ;
	printf( "Concentrator Connected\n" ) ;
	LogPrint( LOG_INFO, "Concentrator Connected\n" ) ;
	CsConnected = 1 ;
	WirelessStatus = 1 ;
	/* Send a radio message WIRELESS_NET_STATUS OK */
	send_wireless_status( RADIO_WIRELESS_NORMAL ) ;
      }
      else {
	int ok ;
	/* Get Cs message and pass to CAN */
	ok = GetCsMsg() ;
	if ( ok <= 0 ) {
	  close( CsConnectedSocket ) ;
	  CsConnectedSocket = -1 ;
	  CsConnected = 0 ;
	  printf( "Concentrator Disconnected\n" ) ;
	  LogPrint( LOG_WARNING, "Concentrator Disconnected\n" ) ;
	  /* Send a radio message WIRELESS_NET_STATUS BAD */
	  send_wireless_status( RADIO_WIRELESS_LISTEN ) ;
	}
	else SendToCan() ;
      }
    }

    if ( FD_ISSET( CanSocket, &rdfs ))  {
      int len ;
      
      FD_CLR( CanSocket, &rdfs ) ;
      len = CanlibReadFrame( &frame ) ;
      if ( len < 0 && errno != EAGAIN ) {
	printf( "Error from Can Socket, byebye\n" ) ;
	LogPrint( LOG_FATAL, "Error reading from Can %d - %s\n", errno, strerror( errno ) ) ;
	Byebye() ;
      }
      else if ( len < 0 && errno == EAGAIN ) break ;
      MsgReceived++ ;
      if ( Verbose > 1 ) dumpFrame( &frame, len ) ;
      /* Pass to Concentrator */
      if ( BuildMessage( &frame, len ) != 0 ) {
	if ( Verbose > 1 )
	  printf( "Complete Message with %d bytes, send to Cs\n",
		  ToCsMsgLength ) ;
	fflush( stdout ) ;
	SendToCs() ;
      }
    }
  }

  return 0 ;
}

/**@}*/
