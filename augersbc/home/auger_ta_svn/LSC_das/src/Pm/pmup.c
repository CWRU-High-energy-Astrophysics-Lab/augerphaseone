#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "msgsvr.h"
#include "central_local.h"

static char * Options = "v?" ;
static char LocalName[2048], RemoteName[128] ;

#define PM_FIFO "/tmp/pm_fifo"
#define UPLOAD_END_FILE "upload.stat"
#define UPLOAD_READY_FILE "upready.stat"

static FILE * FifoOut = NULL ;

static int Nslice = 0 ;

static int Verbose = 0 ;

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
  puts( "pwup <local_path> [<remote_path>]" ) ;
  puts( "Upload a file from LSC" ) ;
  exit( 1 ) ;
}

void HandleOptions( int argc, char ** argv )
{
  int opt ;

  while ( (opt = getopt( argc, argv, Options )) != EOF )
    switch( opt ) {
    case 'v':
      Verbose++ ;
      break ;
    case '?':
    default:
      Help() ;
    }
}

void upload_end()
{
  FILE * fin ;
  char str[16] ;

  fin = fopen( UPLOAD_END_FILE, "r" ) ;
  fgets( str, 16, fin ) ;
  if ( strcmp( str, "OK\n" ) == 0 ) {
    printf( "Upload Finished with %d Slices\n", Nslice ) ;
  }
  else
    printf( "Upload Error\n" ) ;
}

int main( int argc, char ** argv )
{
  int fsize, nslice, total_slice ;
  char * msg ;

  HandleOptions( argc, argv ) ;

  Initialize() ;

  argv++ ;
  if ( *argv == NULL ) {
    printf( "Enter Remote File Name: " ) ;
    fgets( RemoteName, 1024, stdin ) ;
    RemoteName[strlen( RemoteName ) -1] = '\0' ;
  }
  else strcpy( RemoteName, *argv ) ;

  argv++ ;
  if ( *argv != NULL ) {
    strcpy( LocalName, *argv ) ;
  }
  else *LocalName = '\0' ;


  printf( "Remote: '%s', Local: '%s'\n", RemoteName,LocalName) ;

  /* Remove file "upload.stat" */
  unlink( UPLOAD_END_FILE ) ;
  system( "echo Ready >upready.stat" ) ;

  /* Send first Upload message (no message sent to LSC) */

  fprintf( FifoOut, "U %s %s\n", RemoteName, LocalName ) ;
  fflush( FifoOut ) ;

  time_t ttt  ;
  ttt = time( NULL ) ;
  printf( "----> U %s %s - %s", RemoteName, LocalName, ctime( &ttt ) ) ;

  /* Send Upload a slice message until the file "upload.stat" is written
   by Pm. The file should contain just "OK" or "Error" */

  for( Nslice = 0 ; ; ) {

    sleep( 1 ) ;
    if ( access( UPLOAD_END_FILE, 0 ) == 0 ) {
      /* Upload terminated.
	 Read the file to check if finished OK or Error
      */
      upload_end() ;
      break ;
    }
    else if ( access( UPLOAD_READY_FILE, 0 ) != 0 ) {
      if ( Verbose ) printf( "Not Ready\n" ) ;
      continue ;
    }
    else {
      unlink( UPLOAD_READY_FILE ) ;
      fprintf( FifoOut, "u\n" ) ;
      fflush( FifoOut ) ;
      ttt = time( NULL ) ;
      printf( " %d ----> u - %s", Nslice, ctime( &ttt ) ) ;
      Nslice++ ;
    }
  }

  return 0 ;

}
