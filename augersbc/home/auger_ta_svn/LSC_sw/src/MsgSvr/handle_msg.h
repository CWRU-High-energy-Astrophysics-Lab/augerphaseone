/*******************************************

  $CVSAuthor$
  $CVSDate$
  $CVSRevision$

********************************************/

#if !defined(_HANDLE_MSG_H)

#define _HANDLE_MSG_H

typedef struct {
  char name[32] ;
  unsigned int q_key ;
  unsigned int q_id ;
  int nb_id ;
  unsigned char msg_id[128] ;
} MSG_CLIENT_STRUCT ;

int HandleMessage( MSGSVR_PKT * pkt ) ;
#if 0
MSG_CLIENT_STRUCT * GetClientQueue( unsigned char id,
				    MSG_CLIENT_STRUCT * first ) ;
#endif
MSG_CLIENT_STRUCT * GetFirstClient() ;
int IsGoodClient( unsigned char id, MSG_CLIENT_STRUCT * pcl ) ;

#endif
