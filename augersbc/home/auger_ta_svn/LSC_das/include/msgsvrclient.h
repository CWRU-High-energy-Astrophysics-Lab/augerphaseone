#if !defined(_MSGSVRCLIENT_H_)
#define _MSGSVRCLIENT_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-10-20 12:21:42 +0200 (Thu, 20 Oct 2011) $
  $Revision: 1616 $

********************************************/

#define MSGSVR_CLIENT_OPEN_ACTION 0
#define MSGSVR_CLIENT_CLOSE_ACTION 1

typedef struct {
  int action ;
  char name[32] ;
} MSGSVR_CLIENT_HEADER ;

typedef struct {
  MSGSVR_CLIENT_HEADER header ;
} MSGSVR_CLIENT_HEADER_MSG ;

typedef struct {
  MSGSVR_CLIENT_HEADER header ;
  int nb_id ;
  unsigned char msg_id[1] ;
} MSGSVR_CLIENT_OPEN_MSG ;

typedef struct {
  MSGSVR_CLIENT_HEADER header ;
} MSGSVR_CLIENT_CLOSE_MSG ;

int MsgSvrClientVersionNb( void ) ;
char * MsgSvrClientVersion( void ) ;
unsigned int MsgSvrClientOpen( char * qname, int nb_msg,
			       unsigned char * msg_id ) ;
int MsgSvrClientClose( char * qname, unsigned int mqueueid ) ;
int MsgSvrClientSend( int mqueueid, int priority, void * buf, int size ) ;
int MsgSvrClientSignal( char * qname, int signal ) ;
int MsgSvrClientGeneric( int queueid, char * msg, int priority ) ;

#endif
