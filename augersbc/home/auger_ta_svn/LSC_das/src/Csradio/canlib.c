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
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <linux/can.h>

#include "canlib_version.h"

#include "logfile.h"
#include "svrconfig.h"
#include "msgsvr.h"
#include "gpsutil.h"
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

/**
 * read a frame and return frame length
 *
 * @param the_frame contains the data.
 *
 */
int CanlibReadFrame( struct can_frame * the_frame )
{
  int size ;
  if ( CanFd == -1 ) return -1 ;

  memset( the_frame, 0, sizeof( struct can_frame ) ) ;

  size = read( CanFd, the_frame, sizeof( struct can_frame ) ) ;
  if ( size <= 0 ) return size ;
  // Dont know how to test this with can_frame structure !
  //if ( (the_frame->flags ) != 0 && SvrStatus != NULL) SvrStatus->can_overr++ ;

  return the_frame->can_dlc ;
}

int CanlibWriteFrame( struct can_frame * frame, int len, int dbg )
{
  char str[128] ;
  int i, n, ret = 0 ;

  if ( dbg > 1 ) {
    sprintf( str, "   To CAN %03x:", (unsigned int)frame->can_id ) ;
    for ( i = 0 ; i<len ; i++ ) {
      char s[8] ;
      sprintf( s, " %02X", frame->data[i] ) ;
      strcat( str, s ) ;
    }
    LogPrint( LOG_INFO, "%s\n", str ) ;
  }

  /* Now sendto ... */
  if ( CanFd != -1 ) {
    //frame->flags = 0 ;
    frame->can_dlc = len ;
    n = write( CanFd, frame, sizeof( struct can_frame ) ) ;

    if ( n != sizeof( struct can_frame ) ) {
      if ( Verbose ) {
	fprintf( stderr, ">>> Write Error to %d, %d/%d bytes\n",
		 CanFd, n, (unsigned int)sizeof( struct can_frame ) ) ;
	fprintf( stderr, ">>> Error: %s\n", strerror( errno ) ) ;
	fflush( stderr ) ;
      }
      LogPrint( LOG_WARNING, ">>> Write Error to %d, %d/%d bytes\n",
                CanFd, n, sizeof( struct can_frame ) ) ;
      LogPrint( LOG_WARNING, ">>> Error: %s\n", strerror( errno ) ) ;
      ret = -1 ;
    }
    else if ( Verbose )
      LOG_DEBUG( LOG_INFO, "ID %d [0x%x] sent\n", frame->can_id, frame->can_id ) ;
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
  struct can_frame the_frame ;

  /* First CAN id is DATA_COMMAND_UP_BEGIN */
  can_id = first_id ;
  if ( Verbose )
    LOG_DEBUG( LOG_INFO, "UP Data Start, length: %d\n", length ) ;

  for( i = 0, p = msg ; i<length ; can_id += 2, i += CAN_PAYLOAD_LENGTH ) {
    unsigned char *r ;
    int j, last ;

    the_frame.can_id = can_id ;
    last = i + CAN_PAYLOAD_LENGTH ;
    if ( last > length ) {
      last = length ;
      /* Padding */
      memset( the_frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
    }
    for( r = the_frame.data, j = i ; j<last ; j++ )
      *r++ = *p++ ;
#if (CANLIB_VERSION>20)
    int err ;
    if ( (err = CanlibWriteFrame( &the_frame, CAN_PAYLOAD_LENGTH, Verbose )) == 0 ) {
      if ( SvrStatus != NULL ) SvrStatus->can_out++ ;
    }
    else {
      /* Something wrong */
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
  the_frame.can_id = DATA_COMMAND_UP_END ;
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
  struct can_frame the_frame ;

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
    the_frame.can_id = can_id ;
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
  struct can_frame the_frame ;

  /*
    No Padding for LED Controller
    memset( the_frame.data, CAN_PADDING_BYTE, CAN_PAYLOAD_LENGTH ) ;
  */
  the_frame.can_id = can_id ;
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
int CanlibGetDataMsg( struct can_frame * the_frame )
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
#if CANLIB_VERSION>10
int CanlibOpen( int can_fd )
{
  CanFd = can_fd ;
  return CanFd ;
}

void CanlibClose()
{
  CanFd = -1 ;
}
#else
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
#endif



int CanlibFilter( unsigned int id, unsigned int mask )
{
  struct can_filter canfilt ;

  if ( CanFd == -1 ) return -1 ;

  canfilt.can_id = id ;
  canfilt.can_mask = mask ;
#if 0
  if ( ioctl( CanFd, CANQUE_FILTER, &canfilt ) < 0 ) {
    LogPrint( LOG_ERROR, "Cant ioctl filter /dev/can0: %s\n", strerror( errno ) ) ;
    return -1 ;
  }
  else {
    LogPrint( LOG_INFO, "Can Frames filtered: %x, %x\n", id, mask ) ;
    return 0 ;
  }
#endif
}

/**@}*/
