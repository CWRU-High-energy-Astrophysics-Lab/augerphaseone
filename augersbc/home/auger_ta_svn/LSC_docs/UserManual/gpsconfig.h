#if !defined(_GPSCONFIG_H_)

#define _GPSCONFIG_H_ 

/*****************************************************
  $Author: guglielmi $
  $Date: 2011-02-10 11:03:32 +0100 (Thu, 10 Feb 2011) $
  $Revision: 612 $


*****************************************************/

/* Common definitions for GPS related processes */

#include "gpscommon.h"

typedef struct {
  GPS_GEOGRAPHY Geography ;	/**< Position */
  int HoldPositionMode ;    /**< 0 if floating position mode
			       1 if HoldPosition mode 
			       2 if survey mode
			    */
  int Time ;                /**< Default initialisation time */

  unsigned int Htype ;		/**< GPS ellips. reference */

  unsigned int Mask ;       /**< Mask angle degrees  0.. 89  */
  unsigned int AlarmLim ;   /**< Alarm limit in 100s of nsec */
  unsigned int CabDel   ;   /**< Antenna cable delay nsec    */
  int GPSoffset ;		/**< Offset (given by CDAS) */
  int reserved[16] ;        /**< Reserved for future use */
} GPS_CONFIG ;

#define GPS_CONFIG_FILE_NAME "GpsConfig.dat"
#define GPS_CONFIG_DMP_NAME  "GpsConfig.dmp"

#endif
