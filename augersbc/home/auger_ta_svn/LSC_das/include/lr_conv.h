#if !defined(_LR_CONV_H_)
#define _LR_CONV_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-22 12:43:51 #$
  $Revision:: 1779             $

********************************************/


/**
 * @defgroup lr_conv_h Local Radio Monitoring Data Conversion
 * @ingroup services_include
 */
/**@{*/

/**
 * @file   lr_conv.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Sep 21 11:42:43 CEST 2011
 * 
 * @brief  Insert brief description here
*/


/**
 * @struct ONE_LR_STRUCT
 * @brief Name and conversion parameter of the Local Radio Monitoring Data
 *
 */
typedef struct {
  char *name ;			/**< Channel Name */
  int type ;			/**< @brief Float or int */
  float convert ;		/**< Transfert function ( y = x*convert ) */
} ONE_LR_STRUCT ;

#define LR_FLOAT_TYPE 1
#define LR_INT_TYPE 0

/* This structure contains the name, the index in the LrStatus structure
   and the conversion factor to convert ADU to Physica Units
*/
static ONE_LR_STRUCT LrConv[MAX_LR_MONIT_DATA] = {
  /* Frame CGD 1 */
  {"CGD-V0", LR_FLOAT_TYPE, 0.0554},	/**< @brief Input Voltage */
  {"CGD-V1", LR_FLOAT_TYPE, 0.0117},	/**< @brief 1.2V before fuse */
  {"CGD-V2", LR_FLOAT_TYPE, 0.0156},	/**< @brief 3.3V before fuse */
  {"CGD-V3", LR_FLOAT_TYPE, 0.0117},	/**< @brief 1.2V DVDD after fuse */
  {"CGD-V4", LR_FLOAT_TYPE, 0.0156},	/**< @brief 3.3V after fuse */
  {"CGD-V5", LR_FLOAT_TYPE, 0.0140},	/**< @brief 3.0V DVCC */

  /* Frame CGD 2 */
  {"CGD-V6", LR_FLOAT_TYPE, 0.0140},	/**< @brief 3.0V AVDD1 */
  {"CGD-V7", LR_FLOAT_TYPE, 0.0140},	/**< @brief 3.0V AVCC1 */
  {"CGD-V8", LR_FLOAT_TYPE, 0.0119},	/**< @brief 2.5V VCCA */
  {"CGD-T", LR_FLOAT_TYPE, 1.044},		/**< @brief Board temperature */
  /* 2 "reserved for future use" */
  {"-na", LR_INT_TYPE, 0.},
  {"-na", LR_INT_TYPE, 0.},

  /* Frame LR 1 */
  {"CAN-BAD-ID", LR_INT_TYPE, 1.},	/**< @brief 2 bytes las bad CAN Id */
  {"CAN-BAD-UP-DATA", LR_INT_TYPE, 1.},
  {"CAN-BAD-DOWN-DATA", LR_INT_TYPE, 1.},
  {"CAN-BAD-UP-MON", LR_INT_TYPE, 1.},
  {"CAN-BAD-DOWN-MON", LR_INT_TYPE, 1.},

  /* Frame LR 2 */
  {"CAN-BUF-LS", LR_INT_TYPE, 1.},
  {"CAN-BUF-TPCB", LR_INT_TYPE, 1.},
  {"CAN-BUF-UP", LR_INT_TYPE, 1.},
  {"CAN-BUF-DOWN-HI", LR_INT_TYPE, 1.},
  {"CAN-BUF-DOWN-LO", LR_INT_TYPE, 1.},
  /* 1 reserved */
  {"-na", LR_INT_TYPE, 0.},

  /* Frame LR 3 */
  {"BUF-CSMA", LR_INT_TYPE, 1.},
  {"BUF-DOWN-DEF", LR_INT_TYPE, 1.},
  {"BUF-DOWN-MIC", LR_INT_TYPE, 1.},
  {"BUF-DOWN-LO", LR_INT_TYPE, 1.},
  {"BUF-DOWN-GHOST", LR_INT_TYPE, 1.},
  {"BUF-DOWN-ACK", LR_INT_TYPE, 1.},

  /* Frame LR 4 */
  /* 1 reserved */
  {"-na", LR_INT_TYPE, 0.},
  {"BUF-UP-DEF", LR_INT_TYPE, 1.},
  {"BUF-UP-MIC", LR_INT_TYPE, 1.},
  {"BUF-UP-LO", LR_INT_TYPE, 1.},
  {"BUF-UP-GHOST", LR_INT_TYPE, 1.},
  {"BUF-UP-ACK", LR_INT_TYPE, 1.},

  /* Frame LR 5 */
  {"RSSI-T-CS1", LR_INT_TYPE, 1.},
  {"RSSI-T-CS2", LR_INT_TYPE, 1.},
  {"RSSI-F-CS1", LR_INT_TYPE, 1.},
  {"RSSI-F-CS2", LR_INT_TYPE, 1.},
  /* 2 reserved */
  {"-na", LR_INT_TYPE, 0.},
  {"-na", LR_INT_TYPE, 0.},

  /* Frame LR 6 */
  {"RSSI-L-CS1", LR_INT_TYPE, 1.},
  {"RSSI-L-CS2", LR_INT_TYPE, 1.},
  {"RSSI-R-CS1", LR_INT_TYPE, 1.},
  {"RSSI-R-CS2", LR_INT_TYPE, 1.},
  /* 2 reserved */
  {"-na", LR_INT_TYPE, 0.},
  {"-na", LR_INT_TYPE, 0.},

} ;

/* Special to convert the CGD temperature
   temp in degrees = 164.3 + (1.044 * adu)
*/
#define CGD_TEMP_OFFSET 164.3
#define CGD_TEMP_IDX 9

/**@}*/

#endif
