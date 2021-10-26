/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-11 12:29:04 #$
  $Revision:: 916              $

********************************************/

#if !defined(_HANDLE_RADIO_H)

#define _HANDLE_RADIO_H

void SendToRadio( unsigned short id, void * msg, int len ) ;
int CanFromRadio( canmsg_t * frame ) ;

#endif
