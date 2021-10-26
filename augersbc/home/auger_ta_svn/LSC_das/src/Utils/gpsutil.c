/*******************************************

  $Author: guglielmi $
  $Date: 2011-07-20 11:46:23 +0200 (Wed, 20 Jul 2011) $
  $Revision: 1334 $

********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @defgroup gpsutil Utility Functions for GPS
 * @ingroup services_libraries
 *
 */
/**@{*/

#include "gpsutil.h"

unsigned char * short_to_bytes(unsigned char *pb, unsigned short val )
{
  *pb++ = (val >> 8 ) & 0xFF ;
  *pb++ = val & 0xFF ;
  return pb ;
}

short bytes_to_short( unsigned char * pb )
{
  return (*pb << 8) | *(pb+1) ;
}
 
unsigned char *
int_to_bytes( unsigned char *pb, unsigned int val )
{
  *pb++ = (val >> 24) & 0xFF ;
  *pb++ = (val >> 16) & 0xFF ;
  *pb++ = (val >> 8 ) & 0xFF ;
  *pb++ = val & 0xFF ;
  return pb ;
}

unsigned char *
int_to_3bytes( unsigned char *pb, unsigned int val )
{
  *pb++ = (val >> 16) & 0xFF ;
  *pb++ = (val >> 8 ) & 0xFF ;
  *pb++ = val & 0xFF ;
  return pb ;
}

unsigned char *
int_to_2bytes( unsigned char *pb, unsigned int val )
{
  *pb++ = (val >> 8 ) & 0xFF ;
  *pb++ = val & 0xFF ;
  return pb ;
}

int bytes_to_int( unsigned char *pb )
{
  return (*pb << 24) | (*(pb+1)<<16) | (*(pb+2)<<8) | *(pb+3) ;
}

int bytes3_to_int( unsigned char * pb)
{
  return (*(pb)<<16) | (*(pb+1)<<8) | *(pb+2) ;

}

long dswab( char *from )
{
  long value ;

  value = (*from <<24) | (*(from+1)<<16) | (*(from+2)<< 8) | *(from+3) ;
  return value ;
}

double mas_to_deg( int mas )
{
  double value ;

  value = (double)mas/1000. ; // ==> en arcsec
  /* Now into degres */
  return value/3600. ;
}

int deg_to_mas( double deg )
{
  return deg * 3600. * 1000. ;
}

static int Nm[] = {
  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
} ;

unsigned int
gps_seconds( int yy, int mm, int dd, int hh, int mn, int ss )
{
  int sum =0, i = 1, days, leap = 0 ;

  if ( (yy % 4) == 0 ) leap = 1 ;
  for ( ; i < mm ; i++ ) {
    sum += Nm[i] ;
    if ( i == 2 && leap ) sum++ ;
  }
  days = sum + dd - 6 + 365*(yy - 1980) + (yy - 1980 + 3)/4 ;

  return ss + 60*(mn + 60*(hh + 24*days)) ;
}

unsigned int Gps2UtcTime(  unsigned int gps, unsigned int offset )
{
  return gps + GPS_START_TIME - offset ;
}

/** 
 * Returns a string with the UTC time "YYYY/MM/DD hh:mm:ss".
 * 
 * @param gps GPS time in seconds
 * @param offset GPS to UTC offset (given by the GPS receiver)
 * 
 * @return UTC Time as a string
 */
char * Gps2Utc( unsigned int gps, unsigned int offset )
{
  time_t utc ;
  struct tm ttm ;
  static char udate[32] ;

  utc = gps + GPS_START_TIME - offset ;
  ttm = *gmtime( &utc ) ;
  sprintf( udate, "%d/%02d/%02d %02d:%02d:%02d",
	   ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday,
	   ttm.tm_hour, ttm.tm_min, ttm.tm_sec ) ;

  return udate ;
}


/** 
 * Returns a string with the UTC time "YYYYMMDD_hhmmss" that can be used
 * as a file name.
 * 
 * @param gps GPS time in seconds
 * @param offset GPS to UTC offset (given by the GPS receiver)
 * 
 * @return UTC Time as a string
 */
char * Gps2Fname( unsigned int gps, unsigned int offset )
{
  time_t utc ;
  struct tm ttm ;
  static char udate[32] ;

  utc = gps + GPS_START_TIME - offset ;
  ttm = *gmtime( &utc ) ;
  sprintf( udate, "%04d%02d%02d_%02d%02d%02d",
	   ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday,
	   ttm.tm_hour, ttm.tm_min, ttm.tm_sec ) ;

  return udate ;
}

/** 
 * Returns a string with the UTC time "YYYYMMDD" .
 * 
 * @param gps GPS time in seconds
 * @param offset GPS to UTC offset (given by the GPS receiver)
 * 
 * @return UTC Time as a string
 */
char * Gps2UtcYMD( unsigned int gps, unsigned int offset )
{
  time_t utc ;
  struct tm ttm ;
  static char ymd[32] ;

  utc = gps + GPS_START_TIME - offset ;
  ttm = *gmtime( &utc ) ;
  sprintf( ymd, "%04d%02d%02d",
	   ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday ) ;

  return ymd ;
}

/** 
 * Returns a string with the UTC time "YYYY/MM/DD hh:mm:ss" 
 * 
 * @param utc UTC time in seconds
 * 
 * @return UTC Time as a string
 */
char * UtcDateStr( time_t * utc )
{
  struct tm ttm ;
  static char udate[32] ;

  ttm = *gmtime( utc ) ;
  sprintf( udate, "%04d/%02d/%02d %02d:%02d:%02d",
	   ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday,
	   ttm.tm_hour, ttm.tm_min, ttm.tm_sec ) ;

  return udate ;

}

time_t StrDate2Utc( char * date )
{
  struct tm ttm ;
  int yy, mm, dd, hh, min = 0, sec = 0 ;

  sscanf( date, "%d%*c%d%*c%d%*c%d%*c%d%*c%d", &yy, &mm, &dd,
	  &hh, &min, &sec ) ;
  ttm.tm_year = yy - 1900 ;
  ttm.tm_mon = mm -1 ;
  ttm.tm_mday = dd ;
  ttm.tm_hour = hh ;
  ttm.tm_min = min ;
  ttm.tm_sec = sec ;

  return timegm( &ttm ) ;
}

unsigned int Utc2Gps( time_t * utc, int offset )
{

  //printf( "UTC: %d\n", (unsigned int)*utc ) ;

  return *utc - GPS_START_TIME + offset ;
}

/** 
 * Convert a decimal value to its hexa "equivalent".
 * For example: dec = 11, hex = 0x11
 * 
 * @param dec Decimal integer (<255)
 * 
 * @return The hex equivalent
 */
unsigned char dec2hex( int dec )
{
  int yd, yu ;
  yd = dec/10 ;
  yu = dec % 10 ;
  //printf( "yd = %d, yu = %d\n", yd, yu ) ;
  unsigned char yx ;
  yx = (yd*16) + yu ;
  //printf( "yx: %d, 0x%x\n", yx, yx ) ;
  return yx ;
}

/**@}*/
