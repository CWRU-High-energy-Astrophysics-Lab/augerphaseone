#if !defined(_RHTH_DEFS_H_)
#define _RHTH_DEFS_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-06-10 14:00:34 #$
  $Revision:: 1193             $

********************************************/


/**
 * @defgroup rhth_defs_h SHT11 Temperature and Humidity Sensor Definitions
 * @ingroup services_include
 */
/**@{*/

/**
 * @file   rhth_defs.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Jun  1 12:59:50 CEST 2011
 * 
 * @brief  SHT11 Temperature and Humidity Sensor Definitions
*/

#include "hardware.h"

#define THRH_SIZE 0x200
#define THRH_IDENT 0x4F545400
#define THRH_ID_REG 0x100
#define THRH_DATA_REG 0x100
#define THRH_RESET_REG (THRH_DATA_REG+0x8)
#define THRH_DATA_LENGTH 9

#define PCB_TEMPERATURE_NAME "LSC-T"
#define PCB_HYGROMETRY_NAME "LSC-H"

#define THRH_WAIT_MIC 500000

/**@}*/

#endif
