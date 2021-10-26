/*******************************************

  $CVSAuthor$
  $CVSDate$
  $CVSRevision$

********************************************/

#if !defined(_SAVEMSG_H)

#define _SAVEMSG_H
typedef struct {
  int type ;
  int size ;
  int left ;
  int pos ;
  int slice ;
  int msg_nb ;
} SAVED_MESSAGE_HEADER ;

typedef struct {
  SAVED_MESSAGE_HEADER header ;
  unsigned char payload[2] ;
} SAVED_MESSAGE ;

int ToCdasSave( MSGSVR_PKT * pkt ) ;
int ToCdasLeft() ;
void ToCdasSend() ;

#endif
