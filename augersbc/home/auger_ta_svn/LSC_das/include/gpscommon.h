#if !defined(_GPSCOMMON_H_)

#define _GPSCOMMON_H_
/**
 * @defgroup gpscommon_h Structure common to Gpsconfig and Gpsstatus
 * @ingroup gpsconfig_h
 *
 */

/**@{*/
/**
 * @file   gpscommon.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Tue Feb 15 10:40:30 2011
 * 
 * @brief  Sub structures for Gpsconfig and Gpsstatus
 *
 */

#define SERIAL_NUMBER_LENGTH 18

enum {
  TRAIM_SOLUTION_OK,
  TRAIM_SOLUTION_ALARM,
  TRAIM_SOLUTION_UNKNOWN
} ;

typedef struct {
  int lat_mas ;
  int lon_mas ;
  int height ;
} POSITION_MAS ;

typedef struct {
  int northing ;
  int easting ;
  int height ;
  char zone[4] ;
} POSITION_UTM ;

typedef struct {
  int Latitude ;                /**< @brief Lat/Long Coordinates (in MAS) */
  int Longitude ;
  int Altitude ;
  int Northing ;		/**< @brief Northing/Easting Coordinates */
  int Easting ;
  char Zone[4] ;
} GPS_GEOGRAPHY ;


typedef struct {
  int month, day, year ;
  int hour, minute, second ;
  int gmt_offset ;
  int hour_offset ;		/**< Offset (signed) to GMT */
  int minute_offset ;		/**<   ..................    */
} GPS_TIME ;

/* Tout ! */

typedef struct {
  int sat_id ;			/**< Satellite ID */
  int doppler ;			/**< Doppler in Hz */
  int elevation ;		/**< Elevation in degrees */
  int azimuth ;			/**< Azimuth in degrees */
  int health ;			/**< Health - 0 = healthy, 1 = unhealthy */
} SATELLITE_STATUS ;

typedef struct {
  int satellite ;
  int frac_time ;
} SATELLITE_FRACTIONAL ;

typedef struct {
  int svid ;
  int mode ;
  int strength ;
  int iode ;			/**< ????? */
  int status ;
} SATELLITE_DATA ;

typedef struct {
  int pulse ;
  int pps_sync ;		/**< 0 = UTC, 1 = GPS time */
  int traim_solution ;
  int traim_status ;
  unsigned int rem_svid ;
  unsigned short accuracy ;
  char sawtooth ;
  SATELLITE_FRACTIONAL sat_frac[12] ;
} GPS_TRAIM_STATUS ;

/**@}*/

#endif
