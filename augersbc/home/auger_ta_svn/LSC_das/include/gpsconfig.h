#if !defined(_GPSCONFIG_H_)

#define _GPSCONFIG_H_ 

/*****************************************************
  $Author: guglielmi $
  $Date: 2011-03-28 14:47:47 +0200 (Mon, 28 Mar 2011) $
  $Revision: 880 $


*****************************************************/

/* Common definitions for GPS related processes */
/**
 * @defgroup gpsconfig_h Gps Configuration structure
 * @ingroup services_include
 *
 */
/**@{*/

/**
 * @file   gpsconfig.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Tue Feb 15 10:36:29 2011
 * 
 * @brief  Gps Configuration structure
 * 
 * 
 */

#include "gpscommon.h"

typedef struct {
  GPS_GEOGRAPHY Geography ;	/**< @brief Position */
  int HoldPositionMode ;    /**< @brief 0 if floating position mode
			       1 if HoldPosition mode 
			       2 if survey mode
			    */
  int Time ;                /**< @brief Default initialisation time */

  unsigned int Htype ;		/**< @brief GPS ellips. reference */

  unsigned int Mask ;       /**< @brief Mask angle degrees  0.. 89  */
  unsigned int AlarmLim ;   /**< @brief Alarm limit in 100s of nsec */
  unsigned int CabDel   ;   /**< @brief  Antenna cable delay nsec    */
  int GPSoffset ;		/**< @brief  Offset (given by CDAS) */
  int reserved[16] ;        /**< @brief  Reserved for future use */
} GPS_CONFIG ;

#define GPS_LOCATION_FILE_NAME "locations.cfg"

#define GPS_CONFIG_FILE_NAME "GpsConfig.cfg"
#define GPS_CONFIG_DMP_NAME  "GpsConfig.dmp"

/**@}*/

#endif
