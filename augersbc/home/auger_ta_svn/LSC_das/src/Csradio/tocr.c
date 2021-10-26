/*******************************************

  $Author: guglielmi $
  $Date: 2011-09-29 10:23:22 +0200 (Thu, 29 Sep 2011) $
  $Revision: 1579 $

********************************************/

/*
  Description

  Send a message (via message queue) to csradio.

  **********

  History

  V1 - guglielm - 2009/07/07 Creation

*/


/**@{*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "gpsutil.h"
#include "msgqueuelib.h"

char * Qname = NULL ;
unsigned int QueueId = 0xFFFFFFFF;
unsigned int QueueKey = 0 ;

typedef struct {
  long type ;
  int length ;
  char msg[256] ;
} ONE_MSG ;

int get_pid( char * fname )
{
  FILE * fin ;
  int pid = -1 ;

  fin = fopen( fname, "r" ) ;
  if ( fin == NULL ) return -1 ;
  fscanf( fin, "%*s%d", &pid ) ;
  fclose( fin ) ;
  unlink( fname ) ;

  return pid ;
}

void Help()
{
  puts( "tocr: Send a message to csradio" ) ;
  puts( "Usage: tocr <msg>" ) ;
  puts( "Known messages:" ) ;
  puts( "  won   : Send Wireless Normal" ) ;
  puts( "  woff  : Send Wireless OFF" ) ;
  puts( "  ton   : Start sending TPCB Frames" ) ;
  puts( "  toff  : Stop sending TPCB Frames" ) ;
  puts( "  quit  : Quit csradio cleanly" ) ;
  exit( 1 ) ;
}


int main( int argc, char **argv )
{
  ONE_MSG pkt ;
  int size = 0 ;

  // get the signal nb
  if ( argc < 2 ) {
    Help() ;
  }

  Qname = "csradio" ;

  argv++ ;
  for( ; *argv ; argv++ ) {
    QueueId = IpcMsgBind( Qname, &QueueKey ) ;

    if ( QueueId == 0xFFFFFFFF ) {
      fprintf( stderr, "Unknown '%s'\n", Qname ) ;
      continue ;
    }
    else {
      // fill the message
      pkt.type = 1 ;
      pkt.length = strlen( *argv ) + 1 ;
      strcpy( pkt.msg, *argv ) ;
      size = pkt.length + 2*sizeof( int ) ;
      // send the message to the queue
      int ok ;
      ok = msgsnd( QueueId, &pkt, size, IPC_NOWAIT ) ;
    }
  }

  return 0 ;
}

/**@}*/
