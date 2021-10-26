#if !defined(_TTAGLIB_H_)
#define _TTAGLIB_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-04-11 14:11:54 +0200 (Mon, 11 Apr 2011) $
  $Revision: 930 $

********************************************/

#include "gpsstatus.h"
#include "timestamp.h"
#include "ttag.h"

#define TTAG_IN_TRIGGER2 

#define TTAG_DO_CALIB(x,y) (((x) < (y)) ? ((x) + TTAG_NANOS_MAX - (y)) : (x) - (y))

int TtaglibVersionNb() ;
char * TtaglibVersion() ;
void TtaglibSetVerbose( int level ) ;

int TtagOpen() ;
int TtagClose() ;
char TtagGetVersion() ;
int TtagGetId() ;

int TtagClearFifo() ;
int TtagGetFast( unsigned int * sec0, unsigned int * nano0,
		 unsigned int * sec1, unsigned int * nano1 ) ;
int TtagVerifyFast( unsigned int rsec0, unsigned int rnano0,
		    unsigned int  rsec1, unsigned int rnano1,
		    fast_status_t * status, GPS_STATUS * GpsStatus ) ;
int TtagGetSlow( ONE_TIME * date ) ;

unsigned int TtagGetCalib() ;
unsigned int TtagGet100MHz() ;
unsigned int TtagGetSecond() ;
int TtagGetSecondNanos( unsigned int * second, unsigned * nanos ) ;
int TtagReset() ;

#endif
