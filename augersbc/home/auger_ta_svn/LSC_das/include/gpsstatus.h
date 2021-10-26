#if !defined(_GPSSTATUS_H_)

#define _GPSSTATUS_H_

/**
 * @defgroup gpsstatus_h Gps Status structure
 * @ingroup services_include
 *
 */
/**@{*/
/**
 * @file   gpsstatus.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Tue Feb 15 10:55:29 2011
 * 
 * @brief  Structure housing the GPS Status
 * 
 * 
 */

#include "gpscommon.h"

#define GPS_STATUS_NAME "GpsStatus"

#define DEFAULT_ALMANACH_FILE "/root/LSC/config/almanach.cfg"

#define MAX_SAWTOOTH 32
#define SAWTOOTH_MASK 0x1F

typedef struct {
  int calib100,
    raw100 ;
  int cur_time ;
  char sawtooth ;
} SAWTOOTH ;

typedef struct {
    SAWTOOTH table[MAX_SAWTOOTH] ;
} SAWTOOTH_TABLE ;

typedef struct {
  unsigned int initialized ;    /**< @brief 0xBABEFACE if initialized */
  char receiver_full_id[295] ;	/**< @brief Cj - Full receiver ID */
  char receiver_serial[SERIAL_NUMBER_LENGTH+1] ;
  int pps_current_time ;	/**< @brief Current time (from PPS IRQ) */
  int gps_current_time ;	/**< Current time (Incremented by ppsirq) */
  int gps_startup_time ;	/**< @brief Startup time (from GPS receiver) */
  time_t utc_current_time ;	/**< @brief Same as gps_current_time, but UTC */
  int gps_utc_offset ;		/**< @brief Seconds between GPS and UTC time */
  int mask_angle ;
  GPS_TRAIM_STATUS traim_status ; /**< @brief OK, ALARM, UNKNOWN (see gpscommon.h) */
  POSITION_MAS hold_position_mas ;
  POSITION_MAS position_mas ;	/**< @brief Current Position in MAS */
  POSITION_UTM hold_position_utm ;
  POSITION_UTM position_utm ;	/**< @brief Current Position in UTM */
  SAWTOOTH_TABLE sawtooth ;     /**< @brief Sawtooth and calibration table */
  int position_hold ;
  int almanach_status ;
  int almanach_week ;
  int almanach_time ;
  int satellites_update_time ;		/**< @brief GPS time of GpsStatus update */
  int visible_satellites ;	/**< @brief Nb of visible satellites (0-12) */
  int tracked_satellites ;	/**< @brief Nb of tracked ...  */
  SATELLITE_DATA sat_data[12] ;	/**< @brief Ha - Satellite data */
  SATELLITE_STATUS sat_status[12] ;
  int antenna_sense ;
  int leap_second ;		/**< @brief Bj - 0 = no leap second pending */
  GPS_TIME date_time ;
  int traim_enabled ;
  int receiver_offset ;
  /* More to come */
} GPS_STATUS ;

/**@}*/

#endif
