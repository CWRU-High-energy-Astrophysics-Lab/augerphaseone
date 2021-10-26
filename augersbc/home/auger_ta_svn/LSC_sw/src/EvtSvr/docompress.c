#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @defgroup docompress  Compress T3 Events
 * @ingroup event_svr
 *
 */

/**@{*/

#include "logfile.h"

extern int Verbose ;

int DoCompress( void *buf, int size, short evid )
{
  /* Save buf to file
     compress the file
     get the size
     read it back in buf
     return the real length
  */
  int newsize ;
  FILE *file ;
  struct stat stbuf ;
  char bzfname[64], fname[64] ;

  sprintf( fname, "/ram0/t3_%05d", evid ) ;
  sprintf( bzfname, "%s.bz2", fname ) ;

  unlink( bzfname ) ;

  if ( !(file = fopen( fname, "w" )) ) return 0 ;
  fwrite( buf, size, 1, file ) ;
  
  fclose( file ) ;

  if ( Verbose )
    LogPrint( LOG_INFO, "Bzipping '%s', original size: %d\n",
	      fname, size ) ;

  char cmd[64] ;
  sprintf( cmd, "bzip2 %s", fname ) ;
  system( cmd ) ;

  if ( !(file = fopen( bzfname, "r" )) ) {
    if ( Verbose )
      LogPrint( LOG_INFO, "      No bz2 file generated\n" ) ;
    return 0 ;
  }

  fstat( fileno( file ), &stbuf ) ;
  if ( (newsize = stbuf.st_size) > size ) {
    fclose( file ) ;
    if ( Verbose )
      LogPrint( LOG_INFO,
		"Bzip2 compressed file bigger than initial: %d vs %d\n",
		newsize, size ) ;
    return 0 ;
  }
  else   if ( Verbose )
    LogPrint( LOG_INFO, "Compressed file '%s', size: %d\n",
	      bzfname, newsize ) ;

  fread( buf, stbuf.st_size, 1, file ) ;
  fclose( file ) ;

  return newsize ;
}

/**@}*/
