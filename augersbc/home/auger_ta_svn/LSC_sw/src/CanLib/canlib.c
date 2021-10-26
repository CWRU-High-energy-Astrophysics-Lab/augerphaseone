/*******************************************

  $Author: guglielmi $
  $Date: 2011-10-17 09:32:33 +0200 (Mon, 17 Oct 2011) $
  $Revision: 1597 $

********************************************/
/*
  Description


  Split Messages from msgsvr into CAN frames.
  Concatenate CAN Frames form radio into messages to msgsvr.

  **********

  History

  V01 - guglielm - 2009/07/08 Creation
     NOTE: No real CAN There ! Use a standard AF_INET socket for exchange
     with a fakeradio. Should use the "real" can_frame structure to simplify
     the code when using the real CAN driver.
  V02 - LGG - 2010-01-20
     Real CAN now !
  V03 - LGG - 2011-04-01
    Added LEF Specific (NO PADDING) 

  Next versions: see canlib_version.h for history.
    
*/
/**
 * @defgroup canlib  CanBus Library
 * @ingroup  services_libraries
 *
 * Can Bus  Library.
 *
 */
/**@{*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
/* for timeout */
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "can.h"

#include "logfile.h"
#include "svrconfig.h"
#include "msgsvr.h"
#include "gpsutil.h"
#include "canlib_version.h"
#include "candefs.h"
#include "canlib.h"

/**************************************************
  Static Variables
***************************************************/
static char * libVersion = NULL ;
static char * libCvs = "$Author: guglielmi $ - $Revision: 1597 $ - $Date: 2011-10-17 09:32:33 +0200 (Mon, 17 Oct 2011) $" ;
static int CanFd = -1 ;

/**************************************************
  Global Variables
***************************************************/
extern SVR_CONFIG SvrConfig ;
extern int Verbose ;
extern MSGSVR_STATUS * SvrStatus ;

/**************************************************
  Static Functions
***************************************************/

static void setVersionCvs()
{
  libVersion = malloc( 128 ) ;

  sprintf( libVersion, "canlib - V%d.%d\n%s", CANLIB_VERSION/10,
	   CANLIB_VERSION % 10, libCvs ) ;

}

int CanlibReadFrame( canmsg_t * the_frame )
{
  int size ;
  if ( CanFd == -1 ) return -1 ;

  memset( the_frame, 0, sizeof( canmsg_t ) ) ;

  size = read( CanFd, the_frame, sizeof( canmsg_t ) ) ;
  if ( size <= 0 ) return size ;
  if ( (the_frame->flags ) != 0 && SvrStatus != NULL) SvrStatus->can_overr++ ;

  return the_frame->length ;
}

int CanlibWriteFrame( canmsg_t * frame, int len, int dbg )
{
  char str[128] ;
  int i, n, ret = 0 ;

  if ( dbg > 1 ) {
    sprintf( str, "   To CAN %03x:", (unsigned int)frame->id ) ;
    for ( i = 0 ; i<len ; i++ ) {
      char s[8] ;
      sprintf( s, " %02X", frame->data[i] ) ;
      strcat( str, s ) ;
    }
    LogPrint( LOG_INFO, "%s\n", str ) ;
  }

  /* Now sendto ... */
  if ( CanFd != -1 ) {
    frame->flags = 0 ;
    frame->length = len ;
    n = write( CanFd, frame, sizeof( canmsg_t ) ) ;

    if ( n != sizeof( canmsg_t ) ) {
      if ( Verbose ) {
	fprintf( stderr, ">>> Write Error to %d, %d/%d bytes\n",
		 CanFd, n, sizeof( struct canmsg_t ) ) ;
	fprintf( stderr, ">>> Error: %s\n", strerror( errno ) ) ;
		fflush( stderr ) ;
      }
      LogPrint( LOG_WARNING, ">>> Tocdas: Write Error to %d, %d/%d bytes\n",
                CanFd, n, sizeof( struct canmsg_t ) ) ;
      LogPrint( LOG_WARNING, ">>> Tocdas: Error: %s\n", strerror( errno ) ) ;
      ret = -1 ;
    }
    else if ( Verbose )
      LOG_DEBUG( LOG_INFO, "ID %d [0x%x] sent\n", frame->id, frame->id ) ;
  }
  else {
    LogPrint( LOG_WARNING, "CAN Not Initialized\n" ) ;
    ret = -2 ;
  }

  return ret ;
}

/**************************************************
  Global Functions
***************************************************/


int CanlibVersionNb()
{
  return CANLIB_VERSION ;
}

char * CanlibVersion()
{
  if ( libVersion == NULL ) setVersionCvs() ;
  return libVersion ;
}

/** 
 * Send a message to the canbus. The message is splitted into as many
 * CanBus frames as needed, according to the length AND the message type
 * as defined in the include file candefs.h.
 * 
 * @param first_id The message ID (from the canbus point of view)
 * @param msg The message to send to CanBus
 * @param length The size of the message
 * 
 * @return Status (0 if OK)
 */
int CanlibSendDataMsg( int first_id, unsigned char * msg, int length )
{
  int i ;
  unsigned char *p ;
  unsigned short can_id ;
  short slength ;
  canmsg_t the_frame ;

  /* First CAN id is DATA_COMMAND_UP_BEGIN */
  can_id = first_id ;
  if ( Verbose )
    LOG_DEBUG( LOG_INFO, "UP Data Start, length: %d\n", length ) ;

  for( i = 0, p = msg ; i<length ; can_id += 2, i += CAN_PAYLOAD_LENGTH ) {
    unsigned char *r ;
    int j, last ;

    the_frame.id = can_id ;
    last = i + CAN_PAYLOAD_LENGTH ;
    if ( last > length ) {
      last = length ;
      /* Padding */
      memset( the_frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
    }
    for( r = the_frame.data, j = i ; j<last ; j++ )
      *r++ = *p++ ;
#if (CANLIB_VERSION>11)
    int err ;
    if ( (err = CanlibWriteFrame( &the_frame, CAN_PAYLOAD_LENGTH, Verbose )) == 0 ){
      struct timespec t0, t1 ;
      
      if ( SvrStatus != NULL ) SvrStatus->can_out++ ;
      t0.tv_sec = 0 ;
      t0.tv_nsec = 5000000 ;
      nanosleep( &t0, &t1 ) ;       /** Give up a few millis */
    }
    else {
      /* Something wrong ?? */
      return err ;
    }
#else
    if ( CanlibWriteFrame( &the_frame, CAN_PAYLOAD_LENGTH, Verbose ) == 0 &&
	 SvrStatus != NULL )
      SvrStatus->can_out++ ;
    #endif
  }

  /* Send the end of message frame */
  memset( the_frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
  the_frame.id = DATA_COMMAND_UP_END ;
  slength = length ;
  short_to_bytes( the_frame.data, htons( slength ) ) ;
  if ( CanlibWriteFrame( &the_frame, CAN_PAYLOAD_LENGTH, Verbose ) == 0 &&
       SvrStatus != NULL )
    SvrStatus->can_out++ ;
  if ( Verbose )
    LOG_DEBUG( LOG_INFO, "UP Data End\n" ) ;

  return 0 ;
}

/** 
 * Deliver immediately the message to the radio (via canbus).
 * The first short of the message is the CAN Id.
 * 
 * @param msg The message to send to the radio
 * @param length The length of the message (including the ID) 
 * 
 * @return 0 if success, error nb otherwise
 */
int CanlibSendRadioMsg( unsigned char * msg, int length )
{
  int i ;
  unsigned char *p ;
  unsigned short can_id ;
  canmsg_t the_frame ;

  p = msg ;
  can_id = bytes_to_short( p ) ;
  p += 2 ;
  length -= 2 ;
  if ( Verbose )
    LogPrint( LOG_INFO, "CanlibSendRadioMsg: Can_id = %x, length = %d\n",
	      can_id, length ) ;
  for( i = 0 ; i<length ; can_id++, i += CAN_PAYLOAD_LENGTH ) {
    unsigned char *r ;
    int j, last, n ;

    memset( the_frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
    the_frame.id = can_id ;
    last = i + CAN_PAYLOAD_LENGTH ;
    if ( last > length ) last = length ;
    n = last - i ;
    for( r = the_frame.data, j = i ; j<last ; j++ )
      *r++ = *p++ ;
    if ( CanlibWriteFrame( &the_frame, CAN_PAYLOAD_LENGTH,
			   Verbose ) == 0 &&
	 SvrStatus != NULL )
      SvrStatus->can_out++ ;
  }

  return 0 ;
}

/** 
 * Deliver immediately the message to the Led Flasher Controller.
 * 
 * NOTE that there is NO Padding in this case.
 * 
 * 
 * @param[in] can_id CAN ID
 * @param[in] msg The message to send to the Led Flasher Controller
 * @param[in] length The length of the message (including the ID) 
 * 
 * @return 0 if success, error nb otherwise
 */
int CanlibSendLefMsg( int can_id, unsigned char * msg, int length )
{
  int i ;
  unsigned char * r ;
  canmsg_t the_frame ;

  /*
    No Padding for LED Controller
    memset( the_frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
  */
  the_frame.id = can_id ;
  for( r = the_frame.data, i = 0 ; i<length ; i++ )
      *r++ = *msg++ ;
  if ( CanlibWriteFrame( &the_frame, length, Verbose ) == 0 &&
       SvrStatus != NULL )
    SvrStatus->can_out++ ;

  return 0 ;
}

/** 
 * Deliver to MsgSvr one can frame.
 * The message is built locally by concatenation of the CanBus frames
 * as described in the candefs.h include file.
 * 
 * @param the_frame Pointer to Can Frame structure
 * 
 * @return The size of the message
 */
int CanlibGetDataMsg( canmsg_t * the_frame )
{
  int size ;

  if ( Verbose )
    LOG_DEBUG( LOG_INFO, "Go get a CAN frane\n" ) ;

  size = CanlibReadFrame( the_frame ) ;
  if ( SvrStatus != NULL ) SvrStatus->can_in++ ;

  return size ;
}

/** 
 * Open the CAN Socket to the radio (UDP).
 * This is the client ==> Must know the server
 * 
 * 
 * @return The socket nb
 */
int CanlibOpen()
{
  CanFd = open( CAN_DEVICE, O_RDWR ) ;
  return CanFd ;
}

void CanlibClose()
{
  close( CanFd ) ;
  CanFd = -1 ;
}



int CanlibFilter( unsigned int id, unsigned int mask )
{
  struct canfilt_t canfilt = {
    .flags = 0,
    .queid = 0,
    .cob = 0,
    .id = 0,
    .mask = 0
  } ;

  if ( CanFd == -1 ) return -1 ;

  canfilt.id = id ;
  canfilt.mask = mask ;
  if ( ioctl( CanFd, CANQUE_FILTER, &canfilt ) < 0 ) {
    LogPrint( LOG_ERROR, "Cant ioctl filter /dev/can0: %s\n", strerror( errno ) ) ;
    return -1 ;
  }
  else {
    LogPrint( LOG_INFO, "Can Frames filtered: %x, %x\n", id, mask ) ;
    return 0 ;
  }
}

/**@}*/
