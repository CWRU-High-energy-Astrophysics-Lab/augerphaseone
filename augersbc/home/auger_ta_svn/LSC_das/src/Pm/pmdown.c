#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "msgsvr.h"
#include "central_local.h"
#include "download.h"

static char * Options = "?" ;
static char LocalName[2048], RemoteName[128] ;

#define PM_FIFO "/tmp/pm_fifo"

static FILE * FifoOut = NULL ;


static void onlineHelp()
{
}

static void Initialize()
{
  /* Create FIFO */
  char cmd[64] ;

  sprintf( cmd, "mkfifo %s >/dev/null 2>&1", PM_FIFO ) ;
  system( cmd ) ;

  FifoOut = fopen( PM_FIFO, "w+" ) ;
  if ( FifoOut == NULL ) {
    printf( "Cant open Output Fifo '%s'\n", PM_FIFO ) ;
    exit( errno ) ;
  }
}

void Help()
{
  puts( "pwdown <local_path> [<remote_path>]" ) ;
  puts( "Download a file to LSC" ) ;
  exit( 1 ) ;
}

void HandleOptions( int argc, char ** argv )
{
  int opt ;

  while ( (opt = getopt( argc, argv, Options )) != EOF )
    switch( opt ) {
    case '?':
    default:
      Help() ;
    }
}

int main( int argc, char ** argv )
{
  int fsize, nslice, total_slice ;
  char * msg ;

  HandleOptions( argc, argv ) ;

  Initialize() ;

  argv++ ;
  if ( *argv == NULL ) {
    printf( "Enter Local File Name: " ) ;
    fgets( LocalName, 1024, stdin ) ;
    LocalName[strlen( LocalName ) -1] = '\0' ;
  }
  else strcpy( LocalName, *argv ) ;

  printf( "Local File Name '%s'\n", LocalName ) ;
  if ( access( LocalName, 0 ) != 0 ) {
    printf( " ****** UNKNOWN File '%s'\n", LocalName ) ;
    return 1 ;
  }

  argv++ ;
  if ( *argv != NULL ) {
    strcpy( RemoteName, *argv ) ;
  }
  else *RemoteName = '\0' ;

  /* Compute Nb of slices */
  struct stat sbuf ;
  stat( LocalName, &sbuf ) ;
  total_slice = ((sbuf.st_size -1)/DOWN_MAX_SLICE_LENGTH) + 1 ;
  printf( "File Size: %d, nb slices: %d\n", (int)sbuf.st_size,
	  total_slice ) ;

  printf( "Local: '%s', Remote: '%s'\n", LocalName, RemoteName ) ;

  /* Send first download message */
  msg = "D" ;

  fprintf( FifoOut, "%s %s %s\n", msg, LocalName, RemoteName ) ;
  fflush( FifoOut ) ;

  time_t ttt  ;
  ttt = time( NULL ) ;
  printf( "----> %s %s %s - %s", msg, LocalName, RemoteName, ctime( &ttt ) ) ;

  /* Send download a slice message as many times as needed */
  int i ;

  for( i = 0, nslice = 0 ; nslice < total_slice ; ) {

    sleep( 1 ) ;
    if ( i == SLICES_PER_CHECK_POINT ) {
      /* Send checkmsg */
      msg = "dc" ;
      i = 0 ;
    }
    else {
      msg = "ds" ;
      nslice++ ;
      i++ ;
    }

    fprintf( FifoOut, "%s\n", msg ) ;
    fflush( FifoOut ) ;

    ttt = time( NULL ) ;
    printf( " %d ----> %s - %s", nslice, msg, ctime( &ttt ) ) ;
  }

  /* All slices sent, now download check (one more 'd' command) */
    sleep( 1 ) ;
    msg = "dc" ;

    fprintf( FifoOut, "%s\n", msg ) ;
    fflush( FifoOut ) ;

    ttt = time( NULL ) ;
    printf( "----> %s - %s", msg, ctime( &ttt ) ) ;


  return 0 ;

}
