/*******************************************

  $Author: guglielmi $
  $Date: 2011-07-18 15:51:47 +0200 (Mon, 18 Jul 2011) $
  $Revision: 1316 $

  2010-08-17 - LGG
    LogPrint and other, systematically prints the date in seconds.

********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
/**
 * @defgroup logfile  Log Files Library
 * @ingroup services_libraries
 *
 */
/**@{*/
#include "os9_to_lnx.h"
#include "logfile.h"

#define DEFAULT_LOG_FILE_NAME "error.log"

#define LOG_FILE_KEEP_NB 2

static char *LogFileName = NULL ;

static FILE *CurLog= NULL;
static process_id MyPid = 0 ;
static unsigned int *CurrentTime = NULL ;
static int TimedLog = 0, DateLog = 0 ;
static char *LogProgName = NULL ;
static time_t CreationTime = 0 ;
static int DoClose = 1 ;

#define GPS_START_TIME 315964800

#if 0
static char * DateTime( unsigned int gps )
{
  static char date[64] ;
  time_t ttm ;

  ttm = GPS_START_TIME + gps ;
  strcpy( date, ctime( &ttm ) ) ;
  date[strlen(date)-1] = '\0' ;
  return date ;
}

static char * SysDate()
{
  static char date[64] ;
  time_t ttt = time( NULL )  ;
  struct tm ttm ;

  ttm = *gmtime( &ttt ) ;
  sprintf( date, "%4d/%02d/%02d %02d:%02d:%02d",
	   ttm.tm_year + 1900, ttm.tm_mon+1, ttm.tm_mday,
	   ttm.tm_hour, ttm.tm_min, ttm.tm_sec ) ;

  return date ;
}
#endif

static void FileShrink()
{
  /* Reduire la taille d'un fichier en ne gardant que la fin */
  time_t ttt ;
  FILE *fin, *fout ;
  char *buff, *pb ;
  struct stat stbuf ;
  error_code ok ;

  /* Check the size of the file */
  if ( (ok = stat( LogFileName, &stbuf )) != SUCCESS ) return ;

  if ( stbuf.st_size < MAX_LOG_SIZE ) return ;
  /* Save the LEFT_SIZE bytes at the end of the file */
  fin = fopen( LogFileName, "r" ) ;
  if ( fin == NULL ) {
    fprintf( stderr, "Cant open '%s' - '%s'\n",
	     LogFileName, strerror( errno ) ) ;
    return ;
  }
  fseek( fin, -MIN_LOG_SIZE, SEEK_END ) ;
  buff = (char *)malloc( MIN_LOG_SIZE ) ;
  fread( buff, 1, MIN_LOG_SIZE, fin ) ;
  fclose( fin ) ;

  /* Move to an end of line (avoid a line cut in the middle) */
  int size ;
  for( pb=buff, size = MIN_LOG_SIZE ; *pb && *pb != '\n' ; pb++, size-- ) ;

  fout = fopen( LogFileName, "w" ) ;
  if ( fout == NULL ) {
    fprintf( stderr, "Cant open '%s' - '%s'\n",
	     LogFileName, strerror( errno ) ) ;
    return ;
  }
  ttt = time( NULL ) ;
  fprintf( fout, "Logfile '%s' Created - %s", LogFileName,
	   ctime( &CreationTime ) ) ;
  fprintf( fout, "***** Truncated - %s", ctime( &ttt ) ) ;
  fwrite( pb, 1, size, fout ) ;
  fclose( fout ) ;
  free( buff ) ;
}

static void save_previous( const char * fname )
{
  char * yname = malloc( strlen( fname ) + 32 ) ;
  char * zname = malloc( strlen( fname ) + 32 ) ;
  char * cmd = malloc( strlen( fname )*2 + 128 ) ;
  int i ;

  for( i = LOG_FILE_KEEP_NB ; i > 0 ; i-- ) {
    sprintf( yname, "%s.%d.bz2", fname, i-1 ) ;
    sprintf( zname, "%s.%d.bz2", fname, i ) ;
    if ( access( yname, 0 ) == 0 ) {
      sprintf( cmd, "mv %s %s", yname, zname ) ;
      system( cmd ) ;
    }
    //else printf( "   Not found\n" ) ;
  }
  if ( access( fname, 0 ) == 0 ) {
    sprintf( cmd, "mv %s %s.0", fname, fname ) ;
    system( cmd ) ;
    sprintf( cmd, "bzip2 %s.0 &", fname ) ;
    system( cmd ) ;
  }
  free( cmd ) ;
  free( yname ) ;
  free( zname ) ;
}

/*
  Global functions
*/
void LogSetProgName( const char *name )
{
  if ( LogProgName != NULL ) free( LogProgName ) ;
  LogProgName = malloc( strlen( name ) + 1 ) ;
  strcpy( LogProgName, name ) ;
}

void LogInitTime( unsigned int *temps )
{
  CurrentTime = temps ;
}

void LogSetFileName( const char *fname )
{
  LogFileName = malloc( strlen( fname ) + 1 ) ;
  strcpy( LogFileName, fname ) ;
}

void LogSetNewFile( const char *fname )
{
  if ( LogFileName != NULL && strcmp( LogFileName, "stdout" ) == 0 ) return ;

  LogFileName = malloc( strlen( fname) + 1 ) ;
  strcpy( LogFileName, fname ) ;
  //fprintf( stderr, "   LogFileName = %s\n", LogFileName ) ;

  save_previous( fname ) ;

  CreationTime = time( NULL ) ;
  LogPrint( LOG_INFO, "Logfile '%s' Created - %s", fname,
	    ctime( &CreationTime ) ) ;
}

void LogSetNoclose()
{
  DoClose = 0 ;
}

int LogPrt( const char *str, int we, const char * f )
{
  char *swe = "[I]", fun[128] ;
  time_t now = time( NULL ) ;

  if ( MyPid == 0 ) MyPid = getpid() ;

  if ( LogProgName == NULL ) {
    LogProgName = malloc( 8 ) ;
    sprintf( LogProgName, "%d", MyPid ) ;
  }

  if ( LogFileName == NULL ) LogFileName = DEFAULT_LOG_FILE_NAME ;
  if ( strcmp( LogFileName, "stdout" ) != 0 ) FileShrink() ;

  if ( we == LOG_ERROR ) {
    swe = "[E]" ;
  }
  else if ( we == LOG_WARNING ) {
    swe = "[W]" ;
  }
  else if ( we == LOG_FATAL ) swe = "[F]" ;
  else if ( we == LOG_INFO ) swe = "[I]" ;
  else swe = "[U]" ;

  if ( strcmp( LogFileName, "stdout" ) == 0 ) CurLog = stdout ;
  else if ( DoClose == 0 ) {
    /* Open the file Once and do not close */
    if ( CurLog == NULL ) {
      CurLog = fopen( LogFileName, "a+") ;
      //printf( "Open Logfile first time\n" ) ;
    }
    if ( CurLog == NULL ) {
      /* Error logging stopped */
      fprintf( stderr, "Cant open '%s' - %s\n", LogFileName,
	       strerror( errno ) ) ;
      fflush( stderr ) ;
      return 1 ;
    }
  }
  else {
    /* DoClose = 1 --> Open and close file at each print */
    CurLog = fopen( LogFileName, "a+") ;
    if ( CurLog == NULL ) {
      /* Error logging stopped */
      fprintf( stderr, "Cant open '%s' - %s\n", LogFileName,
	       strerror( errno ) ) ;
      fflush( stderr ) ;
      return 1 ;
    }
  }
  if ( f == NULL ) *fun = '\0' ;
  else sprintf( fun, "-%s-", f ) ;

  if ( LogProgName ) fprintf( CurLog, "%s%s(%d)",
			      swe, LogProgName, MyPid ) ;
  else fprintf( CurLog, "%s(%d)", swe, MyPid ) ;

  if ( DateLog == 2 ) {
    fprintf( CurLog, " %u %s : %s", (unsigned int)now, fun, str ) ;
    DateLog = 0 ;
  }
  else if ( !CurrentTime )
    fprintf( CurLog, " %u %s : %s", (unsigned int)now, fun, str ) ;
  else if ( DateLog == 1 )
    fprintf( CurLog, " %u %s : %s",
	     (unsigned int)*CurrentTime, fun, str ) ;
  else if ( TimedLog )
    fprintf( CurLog, " %u %s : %s", (unsigned int)*CurrentTime, fun, str ) ;
  else fprintf( CurLog, " %u %s : %s", (unsigned int)now, fun, str ) ;

  fflush( CurLog ) ;

  if ( strcmp( LogFileName, "stdout" ) == 0 || DoClose == 0 ) return 0 ;
  else {
    fclose( CurLog);
    CurLog = NULL ;
    //printf( "Close Log File\n" ) ;
    return 0;
  }
}

FILE *LogOpen( void )
{
  CurLog = fopen( LogFileName, "a+") ;
  if ( CurLog == NULL ) {
    fprintf( stderr, "Cant open '%s' - %s\n", LogFileName,
	     strerror( errno ) ) ;
    fflush( stderr ) ;
  }
  return CurLog ;
}

void LogClose( void )
{
  fclose( CurLog ) ;
}

/* For some reason, the arm gcc does not find the prototype for vasprintf */
#if defined(FOR_LSC)
void vasprintf( char ** str, const char * fmt, ... ) ;
#endif

int LogDebug( const char * f, int we, const char * format, ... )
{
  va_list args;
  char * the_str ;

  va_start ( args, format);  
  vasprintf ( &the_str, format, args);
  va_end ( args ) ;

  TimedLog = 0 ;
  DateLog = 0 ;
  if ( the_str == NULL ) return LogPrt( "Could not malloc for LogPrint\n",
					we, NULL ) ;
  else {
    int ok = LogPrt( the_str, we, f ) ;
    free( the_str ) ;
    return ok ;
  }
}

int LogPrint( int we, const char *format, ...)
{
  va_list args;
  char * the_str ;

  va_start ( args, format);  
  vasprintf ( &the_str, format, args);
  va_end ( args ) ;

  TimedLog = 1 ;
  DateLog = 0 ;
  /* By default, LogPrt prints the time in seconds */
  if ( the_str == NULL ) return LogPrt( "Could not malloc for LogPrint\n",
					we, NULL ) ;
  else {
    int ok = LogPrt( the_str, we, NULL ) ;
    free( the_str ) ;
    return ok ;
  }
}

int LogPrintTimed( int we, const char *format, ... )
{
  va_list args;
  char * the_str ;

  va_start ( args, format);  
  vasprintf ( &the_str, format, args);
  va_end ( args ) ;

  TimedLog = 1 ;
  if ( the_str == NULL ) return LogPrt( "Could not malloc for LogPrint\n",
					we, NULL ) ;
  else {
    int ok = LogPrt( the_str, we, NULL ) ;
    free( the_str ) ;
    return ok ;
  }
}

int LogPrintDate( int we, const char *format, ... )
{
  va_list args;
  char * the_str ;

  va_start ( args, format);  
  vasprintf ( &the_str, format, args);
  va_end ( args ) ;

  DateLog = 1 ;
  if ( the_str == NULL ) return LogPrt( "Could not malloc for LogPrint\n",
					we, NULL ) ;
  else {
    int ok = LogPrt( the_str, we, NULL ) ;
    free( the_str ) ;
    return ok ;
  }
}

int LogPrintSysDate( int we, const char *format, ... )
{
  va_list args;
  char * the_str ;

  va_start ( args, format);  
  vasprintf ( &the_str, format, args);
  va_end ( args ) ;

  DateLog = 2 ;
  if ( the_str == NULL ) return LogPrt( "Could not malloc for LogPrint\n",
					we, NULL ) ;
  else {
    int ok = LogPrt( the_str, we, NULL ) ;
    free( the_str ) ;
    return ok ;
  }
}
/**@}*/
