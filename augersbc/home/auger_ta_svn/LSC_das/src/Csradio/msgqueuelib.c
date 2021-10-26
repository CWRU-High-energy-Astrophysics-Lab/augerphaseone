#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "util.h"
#include "msgqueuelib.h"

/** 
 * Generate a unique key (unsigned int) from a name. Makes an XOR of all the
 * characters of the name. The key is used to create/link a shared memory, a
 * message queue or a semaphore.
 * 
 * @param name Name of the shared mem segment, msg queue or semaphore.
 * 
 * @return The key generated.
 */
unsigned int mkkey( unsigned char * name )
{
  unsigned int value = 0 ;
  int i ;

  for( i = 0 ; *name != '\0' ; name++, i++ ) {
    unsigned int j = (i % 4)*8 ;
    value ^= (*name << j) ;
  }
  return value ;
}

unsigned int IpcMsgCreate( const char * name, unsigned int * the_key )
{
  unsigned int mykey, msgid ;
  int ok ;
  struct msqid_ds buf ;

  mykey = mkkey( (unsigned char *)name ) ;
  //printf( "Name '%s', Key: %08x\n", name, mykey ) ;

  ok = msgget( mykey, IPC_CREAT | 0x1FF ) ;
  if ( ok == -1 ) exit( 1 ) ;

  msgid = ok ;

  ok = msgctl( msgid, IPC_STAT, &buf ) ;

  buf.msg_perm.mode = 0x1ff ;
  ok = msgctl( msgid, IPC_SET, &buf ) ;

  ok = msgctl( msgid, IPC_STAT, &buf ) ;

  *the_key = mykey ;

  return msgid ;
}

unsigned int IpcMsgBind( const char * name, unsigned int * the_key )
{
  int mykey, ok, msgid ;
  struct msqid_ds buf ;

  mykey = mkkey( (unsigned char *)name ) ;
  //printf( "Name '%s', Key: %08x\n", name, mykey ) ;

  ok = msgget( mykey, 0x1FF ) ;
  if ( ok == -1 ) return ok ;

  msgid = ok ;

  ok = msgctl( msgid, IPC_STAT, &buf ) ;

  buf.msg_perm.mode = 0x1ff ;
  ok = msgctl( msgid, IPC_SET, &buf ) ;

  ok = msgctl( msgid, IPC_STAT, &buf ) ;

  *the_key = mykey ;

  return msgid ;
}

unsigned int IpcMsgCheck( int mykey )
{
  int ok ;

  ok = msgget( mykey, 0x1FF ) ;

  return ok ;
}

int IpcMsgRemove( int msgid )
{
  struct msqid_ds buf ;
  int ok ;

  ok = msgctl( msgid, IPC_RMID, &buf ) ;
  return ok ;
}
