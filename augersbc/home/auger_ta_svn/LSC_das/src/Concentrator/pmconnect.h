/*******************************************

  $Author:: guglielmi          $
  $Date:: 2010-09-20 09:47:28 #$
  $Revision:: 2                $

********************************************/

#if !defined(_PMCONNECT_H)

#define _PMCONNECT_H

#define Pm_MESSAGE_LENGTH		0x28
#define Pm_VERSION				1
#define Pm_BSUID				1
#define Pm_HEADER_LENGTH		16
#define Pm_MESSAGE_VERSION		1
#define Pm_NO_OF_SERVICES		1
#define Pm_SERVICE_ID			1
#define Pm_NO_OF_PARAMS			4
#define Pm_TRAILER				0x1ABCDEF2

int SendConnectionMessage( int bsuid, int socket, struct sockaddr_in * name ) ;

#endif
