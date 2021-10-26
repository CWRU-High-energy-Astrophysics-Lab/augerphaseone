/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-11 12:24:04 #$
  $Revision:: 913              $

********************************************/
/*
  Description



  **********

  History

  V1 - guglielm - 2009/11/13 Creation

*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "pmconnect.h"
#include "logfile.h"

/**************************************************
  Static Variables
***************************************************/

/**************************************************
  Global Variables
***************************************************/
extern int Verbose ;

/**************************************************
  Static Functions
***************************************************/

/**************************************************
  Global Functions
***************************************************/
typedef struct {
  int length ;
  int version ;
  int isbsu ;
  int hlength ;
  int msversion ;
  int nbservices ;
  int servid ;
  int nparams ;
  int bsuid ;
  int trailer ;
} PM_MESSAGE_STRUCT ;

/** 
 * Send to Pm a "connection Message". Should be done prior to any other
 * message. The "normal" data message are a bit different.
 * 
 * @param bsuid BSU ID (dont know what it could be in the RDA)
 * @param socket Socket number
 * @param name socket name
 * 
 * @return 1 if OK, 0 otherwise
 */
int SendConnectionMessage( int bsuid, int socket, struct sockaddr_in * name )
{
  PM_MESSAGE_STRUCT PmMessage ;
  int CharsWritten = 0 ;

  PmMessage.length = htonl(Pm_MESSAGE_LENGTH);
  PmMessage.version = htonl(Pm_VERSION);
  PmMessage.isbsu = htonl(Pm_BSUID);
  PmMessage.hlength = htonl(Pm_HEADER_LENGTH);
  PmMessage.msversion = htonl(Pm_MESSAGE_VERSION);
  PmMessage.nbservices = htonl(Pm_NO_OF_SERVICES);
  PmMessage.servid = htonl(Pm_SERVICE_ID);
  PmMessage.nparams = htonl(Pm_NO_OF_PARAMS);
  PmMessage.bsuid =  htonl((int)bsuid);
  PmMessage.trailer =  htonl(Pm_TRAILER);

  CharsWritten = sendto( socket, &PmMessage, sizeof(PmMessage), 0,
			 (struct sockaddr *)name,
			 sizeof( struct sockaddr_in) ) ;

  int i = 0 ;
  unsigned char *pmsg ;

  pmsg = (unsigned char *)&PmMessage ;

  if ( Verbose )
    LogPrint( LOG_INFO, "SendConnectionMessage: chars written = %d\n",
	      CharsWritten);
  if(CharsWritten == Pm_MESSAGE_LENGTH) {
    LogPrint(LOG_INFO, "Pm connection message Sent\n");
    return 1 ;
  }
  else {
    LogPrint(LOG_WARNING, "Error : Unable to write Pm connection message\n");
    return 0 ;
  }
}
