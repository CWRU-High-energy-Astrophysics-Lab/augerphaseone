/*******************************************

  $CVSAuthor$
  $CVSDate$
  $CVSRevision$

********************************************/

#if !defined(_SAVEMSG_H)

#define _SAVEMSG_H
typedef struct {
  int size ;
  int left ;
} SAVED_MESSAGE_HEADER ;

typedef struct {
  SAVED_MESSAGE_HEADER header ;
  unsigned char payload[2] ;
} SAVED_MESSAGE ;

int SavedMessageSave( MSGSVR_PKT * the_msg, int size ) ;
int SavedMessageLeft() ;
// OBSOLETE void SavedMessageSend() ;
void SavedMessageRest() ;

#endif
