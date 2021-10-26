#if !defined(_CANLIB_H_)
#define _CANLIB_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-03-14 12:48:55 +0100 (Mon, 14 Mar 2011) $
  $Revision: 816 $

********************************************/

#include "canlib_version.h"

int CanlibVersionNb() ;
char * CanlibVersion() ;
int CanlibSendDataMsg( int first_id, unsigned char * msg, int length ) ;
int CanlibSendRadioMsg( unsigned char * msg, int length ) ;
int CanlibSendLefMsg( int id, unsigned char * msg, int length ) ;

int CanlibReadFrame( canmsg_t * frame) ;
int CanlibWriteFrame( canmsg_t * frame, int len, int dbg_flag ) ;
int CanlibGetDataMsg( canmsg_t * frame) ;
#if CANLIB_VERSION>10
int CanlibOpen( int can_fd ) ;
void CanlibClose() ;
#else
int CanlibOpen() ;
void CanlibClose() ;
#endif
int CanlibFilter( unsigned int id, unsigned int mask ) ;

#endif
