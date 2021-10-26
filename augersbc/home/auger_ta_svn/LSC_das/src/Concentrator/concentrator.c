/* 
   concentrator.c
   Created           : 17-MAY-1991 by Laurent Guglielmi

   Establish connection to radiofake (on LSC apcsauron)

   Receive pkts from LSC
   if first pkt, Open connection to Pm

   Add "LSX" header/trailer to pkt.
   Send pkt to Pm

   Receive pkts from CDAS
   Strip off 'LSX' header/trailer
   Send pkt to radiofake

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <getopt.h>

#include "logfile.h"
#include "gpsutil.h"
#include "lsx_defs.h"
#include "pmconnect.h"

static int LsSocket = -1, PmSocket = -1 ;

#define LS_HOST_DEFAULT "localhost"
char * LsHost = NULL ;
int LsPort = 20900 ;

char * PmHost = "localhost" ;
int PmPort = 25200 ;

int Rerror ;
int Debug ;
int Ntries = 0 ;
static 	struct	sockaddr_in ls_sock_name, pm_sock_name ;

int Verbose = 0 ;

int PmConnected = 0, AlreadyDisconnected = 0 ;
int BsuId = 3 ;

enum {
  HELP_OPT, LSHOST_OPT, LSPORT_OPT, PMHOST_OPT, PMPORT_OPT };
char * Options = "v?" ;
struct option longopts[] = {
  {"lshost", required_argument, NULL, LSHOST_OPT},
  {"lsport", required_argument, NULL, LSPORT_OPT},
  {"pmhost", required_argument, NULL, PMHOST_OPT},
  {"pmport", required_argument, NULL, PMPORT_OPT},
  {"help", no_argument, NULL, HELP_OPT},
  {"verbose", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
} ;

#define PM_FIFO "/tmp/pm_fifo"

static FILE * FifoOut = NULL ;

void cleanup( int sock )
{
  shutdown( sock, SHUT_RDWR ) ;
  close( sock ) ;
}

int open_client_socket( char *host, int port, int *sock,
			struct sockaddr_in * sock_name )
{
  /* return 0 in case of timeout, -1 in case of any other socket error,
     1 if OK */
  struct hostent	hostentstruct, *hostentptr;
  int ok ;

  if ( ( *sock = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1) {
    if ( Debug ) perror( "Cant create socket" ) ;
    return -1 ;
  }
  if ( Verbose ) LogPrint( LOG_INFO, "Socket created %d\n", *sock ) ;

  unsigned int the_addr ;
  ok = inet_pton( AF_INET, host, &the_addr ) ;
  //printf( "inet_pton = %d -> The Addr: %x\n", ok, the_addr ) ;

  if ( ok == 1 ) {
    //sin_addr.s_addr = the_addr ;
    /* Host = dd.dd.dd.dd */
    sock_name->sin_family = AF_INET ;
    sock_name->sin_addr.s_addr = the_addr ;
    if ( Verbose ) {
      LogPrint( LOG_INFO, "Found host \"%s\" by addr\n", host ) ;
      //printf( "Found host \"%s\" by addr ==> %x\n", host,
      //      sock_name->sin_addr.s_addr ) ;
    }
  }
  else if ((hostentptr = gethostbyname ( host )) == NULL) {
    perror( "Host Name" ) ;
    return -1 ;
  }
  else {
    hostentstruct = *hostentptr ;
    sock_name->sin_family = hostentstruct.h_addrtype;
    sock_name->sin_addr = * ((struct in_addr *) hostentstruct.h_addr);
    if ( Verbose ) {
      LogPrint( LOG_INFO, "Found host \"%s\" by name\n", host ) ;
      //printf( "Found host \"%s\" by name, addr = %s\n", host,
      //      inet_ntoa( sock_name->sin_addr ) ) ;
    }
  }

  short sport = port ;
  sock_name->sin_port = htons( sport ) ;
  if ( Verbose )
    LogPrintSysDate( LOG_INFO, "Port: %d\n", htons(sock_name->sin_port) ) ;

  Ntries++ ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Trying (%d) to connect on port %d\n", Ntries,
	      port ) ;

  ok = connect ( *sock, (struct sockaddr *)sock_name,
		 sizeof (struct sockaddr) ) ;
  if ( Verbose ) {
    LogPrint( LOG_INFO, "Connect = %d\n", ok ) ;
  }

  int ret = 1 ;

  if ( ok < 0 ) {
    if ( Verbose ) {
      //printf( "Cant connect Error %d: %s\n", errno, strerror( errno ) ) ;
      LogPrint( LOG_ERROR, "Client Can't connect: %s\n",
		strerror( errno ) ) ;
    }
    ret = -1 ;
  }
  else {
    LogPrintSysDate( LOG_INFO, "Connected to %s\n", host ) ;
    ret = 1 ;
  }
  return ret ;
}

void ConnectToLs()
{
  int ntry ;
  int ok ;

  for( ntry = 0 ; ; ntry++ ) {
    if ( LsSocket != -1 ) {
      cleanup( LsSocket ) ;
      LsSocket = -1 ;
    }
    ok = open_client_socket( LsHost, LsPort, &LsSocket, &ls_sock_name ) ;
    if ( ok == 1 ) {
      LogPrint( LOG_INFO, "Connected to LS: '%s' [%d]\n", LsHost, LsSocket ) ;
      fprintf( FifoOut, "Comment***** CS Connected to LSC '%s'\n", LsHost ) ;
      fflush( FifoOut ) ;
      break ;
    }
    else {
      if ( ntry == 0 ) {
	LogPrintSysDate( LOG_FATAL, "Opening LS Socket '%s' Error: %s\n",
			 LsHost, strerror( errno ) ) ;
	fprintf( FifoOut,
		 "Comment***** CS Cant Connect to LSC '%s' Error: %s\n",
		 LsHost, strerror( errno ) ) ;
	fflush( FifoOut ) ;
      }
      sleep( 1 ) ;
    }
  }

}

static void dump_msg( unsigned char * buf, int n )
{
  char * str ;
  char s[8] ;
  int i ;

  LogPrint( LOG_INFO, "Dump msg Pkt - %d bytes\n", n ) ;

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
  free( str ) ;  
}

int SendPktToLs( unsigned char * pkt, int length )
{
  /* Nothing special to do, just send length bytes */
  int n ;

  n = write( LsSocket, pkt, length ) ;
  if ( n != length ) {
    LogPrint( LOG_WARNING, "Can't write full frame to LS: %d vs %d\n",
	      n, length ) ;
  }
  else LogPrint( LOG_INFO, "%d bytes sent to LS\n", n ) ;
  return n ;
}

/*
  Get pkt from Pm.
  Strip off header and CRC
  NOTE: The CRC is NOT verified here ! Anyway why should we do that ? We are
  using TCP connection with PM, which is a rather secure way to send and 
  receive data.

  The packet received from Pm is made of:
    Preamble: "  !PC2BS!"
    Length: 2 bytes, the overall length, excluding itself
    Type: 1 byte = 'D' character
    BSID: 1 byte. (the Concnetrator destination ?)
    The "payload": the frame as it is sent to LSC
    The CRC: 4 bytes
    The Trailer: 1 byte = 0xFF
*/
unsigned char * GetPmPkt( int * npm )
{
  int n ;
  short length ;
  char preamble[12] ;
  unsigned char ltb[LTB_LENGTH] ;
  unsigned char * buf ;

  /* Read preamble */
  n = read( PmSocket, preamble, PM_TO_BS_PREAMBL_LENGTH ) ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Preamble read: %d/%d\n", n, PM_TO_BS_PREAMBL_LENGTH ) ;
  if ( n != PM_TO_BS_PREAMBL_LENGTH ) {
    *npm = -1 ;
    return NULL ;
  }
  preamble[PM_TO_BS_PREAMBL_LENGTH] = '\0' ;
  LogPrint( LOG_INFO, "Preamble '%s'\n", preamble ) ;
  /* Read Size, type, BsId */
  n = read( PmSocket, ltb, 2 ) ;
  if ( Verbose ) LogPrint( LOG_INFO, "Pkt Length read: %d\n", n ) ;
  if ( n != 2 ) {
    *npm = -1 ;
    return NULL ;
  }
  length = bytes_to_short( ltb ) ;
  if ( Verbose ) LogPrint( LOG_INFO, "Length: %d\n", length ) ;
  unsigned char datum, bsid ;

  n = read( PmSocket, &datum, 1 ) ;
  n = read( PmSocket, &bsid, 1 ) ;
  if ( Verbose ) LogPrint( LOG_INFO, "Datum=0x%02x, Bsid=0x%02x\n", datum, bsid ) ;

  /* 
     payload_size is the actual size of the frame to be sent to LSC "as  it is"
     Suppress from Pm pkt length:
       Datum (1 byte), bsid (1 byte), CRC (4 bytes) and ETX (1 byte)
  */
  int payload_size = length - CRC_LENGTH - DATUM_BYTE_LENGTH -
    BSID_BYTE_LENGTH - ETX_BYTE_LENGTH;
  LogPrint( LOG_INFO, "   Payload size: %d\n", payload_size ) ;
  /* Read the rest of the packet (LS frame + CRC + ETX)*/
  buf = malloc( length ) ;
  n = read( PmSocket, buf, payload_size + CRC_LENGTH + 1 ) ;
  dump_msg( buf, payload_size + CRC_LENGTH + 1 ) ;
  if ( Verbose )
    LogPrint( LOG_INFO, "Read Rest of message: %d/%d\n",
	    n, payload_size + CRC_LENGTH + 1 ) ;
  if ( n != (payload_size + CRC_LENGTH + 1) ) {
    *npm = -1 ;
    free( buf ) ;
    return NULL ;
  }

  /* Suppress CRC */
  *npm = payload_size ;
  return buf ;
}

/*
  PM Acknowledge Message:
  Al items are 32 bits integer

  Mess.pmStdHeader.length = PmACKMESSLENGTH;
  Mess.pmStdHeader.protocolVersion = GsVERSION;	// RR - using macro
  Mess.pmStdHeader.type = GsACK;	// RR - using macro
  Mess.pmStdHeader.hLength = PmSTDHEADERLENGTH;
  Mess.version = 1;		// RR ? where is the macro for this!!
  Mess.errorCode = code;
  Mess.trailer.id = PmMESSAGESENDMARKER;
*/

int waitPmAck()
{
  /* Read the ack message */
  unsigned int Ack[28] ;
  int n = 0, j = 0, length = sizeof( Ack ) ;

  if ( Verbose ) {
    LogPrint( LOG_INFO, "Wait Ack, %d bytes\n", length ) ;
  }
  for ( ; ; ) {
    j = read( PmSocket, Ack, sizeof( Ack ) ) ;
    n += j ;
    if ( Verbose )
      LogPrint( LOG_INFO, "Got Ack %d, expected %d\n", n, length ) ;
    if ( n == length ) break ;
  }
  LogPrint( LOG_INFO, "Ack Received from Pm\n" ) ;

  return 0 ;
}

int ConnectToPm()
{
  int ok ;

  if ( Verbose ) LogPrint( LOG_INFO, "ConnectToPm:\n" ) ;

  ok = open_client_socket( PmHost, PmPort, &PmSocket, &pm_sock_name ) ;
  if ( ok < 0 ) {
    if ( Verbose > 1 )
      LogPrint( LOG_WARNING, "Opening Pm socket '%s' Error: %s\n", PmHost,
		strerror( errno ) ) ;
    cleanup( PmSocket ) ;
    return ok ;
  }
  else LogPrintSysDate( LOG_INFO, "Pm Connected, socket #: %d\n",
			PmSocket ) ;

  //  partir de la, on est connecte, encore faut-il se presenter correctement
  // a Pm pour qu'il soit content.
  ok = SendConnectionMessage( BsuId, PmSocket, &pm_sock_name ) ;

  /* Now wait till got an acknowledge */
  waitPmAck() ;

  return ok ;
}

/** 
 * Send a packet to Pm.
 * Add the header and the CRC and the end of pkt marker (ETX).
 * CRC is calculated on the packet except the preamble ( ie including length,
 * type and BsId)
 *
 * Do we really have to add a CRC and an ETX ???? The pkt is send over the
 * network using TCP !!!
 *
 * A BS packet is made of:
 *
 *    - BS Pkt
 *       - Preambule = "!BS2PC!"
 *       - Length (2 bytes): Length of the rest, excepting itself.
 *       - Type ( 1 byte): 0x44 = 'D'
 *       - BsID ( 1 Byte)
 *       - LS Frame
 *       - CRC (4 bytes): on everything including length (excluding the
 *           CRC of course. Cant understand why we need that !
 *       - ETX (1 byte): 0xFF
 *    - LS Frame
 *       - Length ( 2 bytes): All the frame except itself
 *       - FrameNb (1 byte): Frame number
 *       - Reserved (1 byte): 0x00
 *       - LsId (2 bytes):
 *       - Nb of Messages (1 byte): Nb of messages in the frame.
 *       - The LS Messages:
 *    - LS Message
 *       - Length (2 bytes): all the message including itself
 *       - Slice (1 byte): slice completion (2 bits) + Slice number
 *       - Type ( 1 byte): Message type
 *       - Message Nb (1 byte): message nb (6 bits) + version (2 bits)
 *       - The message payload (Length - 5 bytes)
 *
 * 
 * @param buf Input data packet
 * 
 * @return OK
 */
int SendPktToPm( unsigned char * buf )
{
  unsigned char wbuf[1024], *pwbuf ;
  short lsc_size, pkt_size ;
  int len ;
  unsigned long int the_crc ;

  // get the size of the pkt received from LSC
  lsc_size = bytes_to_short( buf ) + 2 ;
  if ( Verbose ) LogPrint( LOG_INFO, "LSC Size: %d\n", lsc_size ) ;
  pkt_size = lsc_size + BS_TO_PM_PREAMBLE_LENGTH + LTB_LENGTH + CRC_LENGTH + 1 ;
  memcpy( wbuf, BS_TO_PM_PREAMBLE, BS_TO_PM_PREAMBLE_LENGTH ) ;
  pwbuf = wbuf + BS_TO_PM_PREAMBLE_LENGTH ;
  /*
    Excluded from length: header, length and the final ETX */
  pwbuf = short_to_bytes( pwbuf, lsc_size + DATA_LENGTH_MODIFIER + 1 ) ;
  *pwbuf++ = 'D' ; /* Data Type */
  *pwbuf++ = BsuId ;
  memcpy( pwbuf, buf, lsc_size ) ;
  pwbuf += lsc_size ;

#if 0
  /* Now add the CRC, calculated on the data including length, type and bsid */
  the_crc = CRC_32bit( wbuf + BS_TO_PM_PREAMBLE_LENGTH, lsc_size + 4 ) ;
#else
  /* Dont mind the CRC ! */
  the_crc = 0 ;
#endif
  pwbuf = int_to_bytes( pwbuf, the_crc ) ;
  /* And the End of pkt marker */
  *pwbuf = ETX;

  if ( Verbose )
    LogPrint( LOG_INFO, "lsc_size: %d, pkt_size: %d\n", lsc_size, pkt_size ) ;

  // Send
  struct timeval tm ;
  gettimeofday( &tm, NULL ) ;
  LogPrint( LOG_INFO, " Send Pkt to Pm at %d.%06d\n",
	    tm.tv_sec, tm.tv_usec ) ;
  len = write( PmSocket, wbuf, pkt_size ) ;
  if ( len != pkt_size ) {
    LogPrint( LOG_WARNING, "Write to socket Error: %s\n",
	      strerror( errno ) ) ;
    cleanup( PmSocket ) ;
  }
  else {
    LogPrint( LOG_INFO, "Pkt sent to Pm, %d bytes, payload: %d bytes\n",
	      len, lsc_size ) ;
    if ( Verbose ) {
      LogPrint( LOG_INFO, "   CRC: %x\n", the_crc ) ;
    }
    dump_msg( wbuf, pkt_size ) ;
  }
  return len ;
}

static void Help()
{
  puts( "concentrator <options>" ) ;
  puts( "Options:" ) ;
  puts( " --lshost=<host>  : LSC Server Host name (default 'localhost')" ) ;
  puts( " --lsport=<port>  : LSC TCP Port (default 20900)" ) ;
  printf( " --pmhost=<host>  : Pm Server Host name. Default '%s'\n", PmHost ) ;
  printf( " --pmport=<port>  : Pm TCP Port (default %d)\n", PmPort ) ;
  puts( " --help           : What you see" ) ;
  puts( " --verbose        : Increase verbosity" ) ;
  exit( 1 ) ;
}

static void HandleOptions( int argc, char **argv )
{
  int opt ;

  while( (opt = getopt_long( argc, argv, Options, longopts, NULL )) != EOF )
    switch ( opt ) {
    case LSHOST_OPT:
      LsHost = malloc( strlen( optarg ) + 1 ) ;
      strcpy( LsHost, optarg ) ;
      break ;
    case LSPORT_OPT:
      sscanf( optarg, "%d", &LsPort ) ;
      break ;
    case PMHOST_OPT:
      PmHost = malloc( strlen( optarg ) + 1 ) ;
      strcpy( PmHost, optarg ) ;
      break ;
    case PMPORT_OPT:
      sscanf( optarg, "%d", &PmPort ) ;
      break ;
    case 'v':
      Verbose++ ;
      break ;
    case '?':
    case 'h':
    case HELP_OPT:
      Help() ;
    default:
      Help() ;
    }
}

void got_sig( int sig )
{
  switch ( sig ) {
  case SIGINT:
  case SIGQUIT:
    LogPrint( LOG_FATAL, "Killed by signal %d\n", sig ) ;
    cleanup( PmSocket ) ;
    exit( 0 ) ;
  default:
    LogPrint( LOG_WARNING, "Unexpected signal %d\n", sig ) ;
    break ;
  }
}

int main( int argc, char ** argv )
{
  int max_fd = 3 ;		/**< at least 3 descriptos automatically opened */
  fd_set rdfs, wrfs, erfs ;
  struct timeval tmout ;
  int no_connect = 0 ;

  HandleOptions( argc, argv ) ;

  LogSetProgName( "cs" ) ;
  LogSetNewFile( "cs.log" ) ;
  LogPrintSysDate( LOG_INFO, "******* Starting (pid = %d)\n", getpid() ) ;

  signal( SIGQUIT, got_sig ) ;
  signal( SIGINT, got_sig ) ;

  FD_ZERO( &rdfs ) ;
  FD_ZERO( &wrfs ) ;
  FD_ZERO( &erfs ) ;

  if ( Verbose ) LogPrint( LOG_INFO, "Connecting to Pm\n" ) ;
  while ( ConnectToPm() != 1 ) {
    if ( no_connect == 0 ) {
      LogPrint( LOG_INFO, "Cant Connect to Pm '%s' - Wait and Retry\n", PmHost ) ;
      no_connect = 1 ;
    }
    sleep( 2 ) ;
  }
  no_connect = 0 ;
  PmConnected = 1 ;

  char cmd[128] ;
  sprintf( cmd, "mkfifo %s >/dev/null 2>&1", PM_FIFO ) ;
  system( cmd ) ;

  FifoOut = fopen( PM_FIFO, "w+" ) ;

  if ( LsHost == NULL ) LsHost = LS_HOST_DEFAULT ;
  ConnectToLs() ;
    
  if ( Verbose ) LogPrintSysDate( LOG_INFO, "LS Socket: %d\n", LsSocket ) ;

  //  if ( LsSocket >= max_fd ) max_fd = LsSocket + 1 ;
  max_fd = (LsSocket>PmSocket)?(LsSocket+1):(PmSocket+1) ;

  for( ;; ) {
    int ok ;

    FD_SET( LsSocket, &rdfs ) ;
    if ( PmConnected ) {
      FD_SET( PmSocket, &rdfs ) ;
    }

    tmout.tv_sec = 0 ;
    tmout.tv_usec = 10000 ;	/**< Waiting 10 millis */

    ok = select( max_fd, &rdfs, NULL, NULL, &tmout ) ;
    if ( ok == 0 ) {
      /* Time out expired, do nothing */
      continue ;
    }
    else if ( ok < 0 ) {
      LogPrintSysDate( LOG_FATAL, "Select Error: %s\n", strerror( errno ) ) ;
      exit( 1 ) ;
    }
    else {
      if ( FD_ISSET( LsSocket, &rdfs ) ) {
	/* Got something from LS */
	unsigned char * buf = NULL ;
	int n, len ;

	FD_CLR( LsSocket, &rdfs ) ;

	n = read( LsSocket, &len, sizeof(short) );
	if ( n != sizeof( short ) ) {
	  LogPrintSysDate( LOG_FATAL, "Connection Error: %s\n", strerror( errno ) ) ;
	  // close LS connection and re-open it
	  cleanup( LsSocket ) ;
	  LsSocket = -1 ;
	  ConnectToLs() ;
	  if ( PmSocket >= max_fd ) max_fd = PmSocket + 1 ;
	  continue ;
	}
	len = ntohs( len ) ;
	if ( Verbose ) {
	  LogPrintSysDate( LOG_INFO, "Got a message from LS, %d bytes\n", len ) ;
	}
	buf = malloc( len + 2 ) ;
	short_to_bytes( buf, len ) ;

	n = read( LsSocket, buf+2, len ) ;

	if ( n <= 0 ) {
	  LogPrintSysDate( LOG_FATAL, "Connection Error [%d bytes]: %s\n", n,
			   strerror( errno ) ) ;
	  // close LS connection and re-open it
	  ConnectToLs() ;
	  if ( PmSocket >= max_fd ) max_fd = PmSocket + 1 ;
	  continue ;
	}
	else {
	  short pktln =  bytes_to_short( buf ) ;
	  if ( Verbose)
	    LogPrint( LOG_INFO, "Pkt Length: %u, LSid: %u\n",
		      bytes_to_short( buf ),
		      bytes_to_short( buf+4) ) ;
	  unsigned char pktnb ;
	  pktnb = *(buf+2) ;
	  LogPrint( LOG_INFO, "  Pkt #: %d\n", pktnb ) ;
	  /* Should check the pkt, sometimes there are silly things there ! */
	  if ( pktln > 288 ) {
	    LogPrint( LOG_ERROR, "Bad Pkt length: %d (0x%x)\n",
		      pktln, pktln ) ;
	    dump_msg( buf, 64 ) ;
	    LogPrint( LOG_ERROR, " ....  Ignore !\n" ) ;
	  }
	  else if ( PmConnected == 0 ) {
	    if ( ConnectToPm() == 1 ) {
	      PmConnected = 1 ;
	      AlreadyDisconnected = 0 ;
	      if ( PmSocket >= max_fd ) max_fd = PmSocket + 1 ;
	      LogPrintSysDate( LOG_INFO, "Connected to Pm\n" ) ;
	      SendPktToPm( buf ) ;
	    }
	    else {
	      if ( AlreadyDisconnected == 0 ) {
		AlreadyDisconnected = 1 ;
		LogPrintSysDate( LOG_WARNING, "Can't Connect to Pm: %s\n",
				 strerror( errno ) ) ;
	      }
	    }
	  }
	  else SendPktToPm( buf ) ;
	}
	if ( buf != NULL ) free( buf ) ;
      }
      else if ( PmConnected && FD_ISSET( PmSocket, &rdfs ) ) {
	int npm ;
	unsigned char * tols_msg ;

	FD_CLR( PmSocket, &rdfs ) ;
	LogPrintSysDate( LOG_INFO, "Got Pkt from Pm\n" ) ;
	tols_msg = GetPmPkt( &npm ) ;
	if ( npm <= 0 ) {
	  LogPrint( LOG_WARNING, "Pm Disconnected, try again later\n" ) ;
	  PmConnected = 0 ;
	  cleanup( PmSocket ) ;
	  continue ;
	}
	else {
	  LogPrint( LOG_INFO, "Got %d bytes from Pm\n", npm ) ;
	  /* Send to LS */
	  SendPktToLs( tols_msg, npm ) ;
	  free( tols_msg ) ;
	}
      }
    }
  }

  return 0 ;
}
