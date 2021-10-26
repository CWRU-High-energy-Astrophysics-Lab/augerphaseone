#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define TOPM_VERSION 11

static char * Options = "?" ;
static int Interactive = 1 ;

#define PM_FIFO "/tmp/pm_fifo"

static FILE * FifoOut = NULL ;

static void onlineHelp()
{
  printf( "\nPm Commands:\n" ) ;
  printf( "  c <cmd>\t: shell cmd\n" ) ;
  printf( "  G <msg>\t: Generic msg,\n" ) ;
  printf( "  Ga\t\t: Generic message 'acqstatus -v'\n" ) ;
  printf( "  +/-\t\t: verbose\n" ) ;
  printf( "  Rs\t\t: reStart Services\n  d/D\t\t: download\n  u/U\t\t: upload\n" ) ;
  printf( "  l/L/A/*\t: Single/List/Antilist/Broadcast\n" ) ;
  printf( "  w\t\t: WakeUp\n" ) ;
  printf( "  T3\t\t: T3_YES request (latest T2 second and micros)\n" ) ;
#if TOPM_VERSION>10
  printf( "  T3,second,micros,window[,evid]: T3_YES request with second, etc. evid is optional\n" ) ;
#endif
  printf( "  A3\t\t: T3_YES request Same Again\n" ) ;
  printf( "  S3\t\t: T3_YES request Same\n" ) ;
  printf( "  M\t\t: Monit Request\n" ) ;
  printf( "  Mv\t\t: Toggle Monit print\n" ) ;
  printf( "  Ra\t\t: ReStart acquisition\n" ) ;
  printf( "  S/s\t\t: Start/stop acquisition\n  T/t\t\t: Start/stop triggers\n" ) ;
  printf( "  St/st\t\t: Start/stop Saving T2\n" ) ;
  printf( "  v\t\t: Toggle T2 print\n  Reboot\t: Reboot the LSC\n" ) ;
  printf( "  q\t\t: Stop Pm\n" ) ;
  printf( "Topm Commands:\n  x\t\t: eXit topm\n" ) ;
  printf( "  ?\t\t: This help\n" ) ;

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
  puts( "topm [<cmd>]" ) ;
  puts( "Send commands to Pm" ) ;
  puts( "Usage:" ) ;
  puts( " topm <cmd> : Send the command <cmd> and quit" ) ;
  puts( " topm       : Interactive" ) ;
  puts( "Interactive commands:" ) ;
  onlineHelp() ;
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
  char str[128] ;

  HandleOptions( argc, argv ) ;

  Initialize() ;

  if ( argc != 1 ) {
    Interactive = 0 ;
    argv++ ;
  }

  if ( Interactive ) for( ; ; ) {
    printf( "Enter Pm Command: " ) ;
    fgets( str, 128, stdin ) ;
    str[strlen( str ) -1] = '\0' ;
    printf( "===>%s<===\n", str ) ;
    if ( *str == '?' ) onlineHelp() ;
    else if ( *str == 'x' ) {
      /* Quit topm */
      exit( 0 ) ;
    }
    else if ( strcmp( str, "Ga" ) == 0 ) {
      fprintf( FifoOut, "G acqstatus -v\n" ) ;
      fflush( FifoOut ) ;
    }
    else {
      /* Write the cmd to the Fifo */
      fprintf( FifoOut, "%s\n", str ) ;
      fflush( FifoOut ) ;
    }
  }
  else for( ; *argv != NULL ; argv++ ) {
    /* Write the cmd to the Fifo */
    printf( " -->%s<--\n", *argv ) ;
    if ( strcmp( *argv, "Ga" ) == 0 ) {
      fprintf( FifoOut, "G acqstatus -v\n" ) ;
      fflush( FifoOut ) ;
    }
    else {
      fprintf( FifoOut, "%s\n", *argv ) ;
      fflush( FifoOut ) ;
    }
    sleep( 1 ) ;
  }


  return 0 ;

}
