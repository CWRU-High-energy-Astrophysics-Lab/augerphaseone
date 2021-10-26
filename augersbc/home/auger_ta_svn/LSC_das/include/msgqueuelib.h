#if !defined(_MSGQUEUELIB_H_)
#define _MSGQUEUELIB_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2010-09-20 09:49:56 +0200 (Mon, 20 Sep 2010) $
  $Revision: 4 $

********************************************/

unsigned int IpcMsgCreate( const char * name, unsigned int * key ) ;
unsigned int IpcMsgBind( const char * name, unsigned int * key ) ;
int IpcMsgRemove( int msgid ) ;
unsigned int IpcMsgCheck( int msqid ) ;

#endif
