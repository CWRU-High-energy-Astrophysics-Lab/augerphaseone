#if !defined(_GPSUTIL_H_)

#define _GPSUTIL_H_
/**
 * @defgroup gpsutil_h Time related definitions
 * @ingroup services_include
 */

/**@{*/

#define GPS_START_TIME 315964800

unsigned char * short_to_bytes(unsigned char *pb, unsigned short val ) ;
short bytes_to_short( unsigned char * pb ) ;
unsigned char * int_to_bytes( unsigned char *pb, unsigned int val ) ;
unsigned char *
int_to_3bytes( unsigned char *pb, unsigned int val ) ;
unsigned char *
int_to_2bytes( unsigned char *pb, unsigned int val ) ;

int bytes_to_int( unsigned char *pb ) ;
int bytes3_to_int( unsigned char * pb) ;

long dswab( char *from ) ;
double mas_to_deg( int mas ) ;
int deg_to_mas( double deg ) ;
unsigned int
gps_seconds( int yy, int mm, int dd, int hh, int mn, int ss ) ;

char * Gps2Utc( unsigned int gps, unsigned int offset ) ;
unsigned int Gps2UtcTime(  unsigned int gps, unsigned int offset ) ;
char * Gps2Fname( unsigned int gps, unsigned int offset ) ;
char * Gps2UtcYMD( unsigned int gps, unsigned int offset ) ;
time_t StrDate2Utc( char * date ) ;
unsigned int Utc2Gps( time_t * utc, int offset ) ;
char * UtcDateStr( time_t * utc ) ;
unsigned char dec2hex( int dec ) ;

/**@}*/

#endif

