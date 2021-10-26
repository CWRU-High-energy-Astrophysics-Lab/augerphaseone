#if !defined(_TTAG_H_)
#define _TTAG_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-04-11 14:12:23 +0200 (Mon, 11 Apr 2011) $
  $Revision: 931 $

********************************************/

#include "hardware.h"

#define TTAG_CLOCK_PERIOD 100000000

#define TTAG_CALIB_MASK   0xFFFFFFF
#define TTAG_NANOS_MASK   0x7FFFFFF /**< 27 bits */
#define TTAG_COUNT_MASK   0xF0000000 /**< @brief Evt counter (4 bits) */
#define TTAG_NANOS_MAX    0x8000000
#define TTAG_SECOND_MASK  0xFFFFFFF /**< 28 bits */
#define TTAG_SECOND_MAX   0x10000000
#define TTAG_SECOND_LOG_MASK 0xF0000000 /**< @brief log */

#define TTAG_EDGE_MASK     0x8000000
#define TTAG_LEADING_EDGE  0x0000000
#define TTAG_TRAILING_EDGE 0x8000000
#define TTAG_GOOD_EDGES    0x1

#define TTAG_SIZE         0x200

#define TTAG_CALIB_OFFSET           0x0 /**< Calibration 100MHz (27bits) */
#define TTAG_EVTCLKF_SECOND_OFFSET  0x1 /**< Event fast second  (28bits)  */
#define TTAG_EVTCLKF_NANO_OFFSET    0x2 /**< Event fast nano (27bits) */

#define TTAG_EVTCLKS_SECOND_OFFSET  0x3 /**< Muon second  (28bits)  */
#define TTAG_EVTCLKS_NANO_OFFSET    0x4 /**< Muon nano (27bits) */

#define TTAG_SECOND_OFFSET          0x5  /**< Compteur 1PPS  (28bits) */
#define TTAG_100MHZ_OFFSET          0x6  /**< Compteur 100MHz  (27bits) */
#define TTAG_ID_OFFSET              0x7  /**< TTAG_ID = $545456xx  */
#define TTAG_VERSION_OFFSET         TTAG_ID_OFFSET

#define PREV_TTAG_ID_NAME           0x54544100 /**< Previous ID */
#define TTAG_ID_NAME                0x54545600 /**< "TTV" + 1 ASCII
						  char as version number */
#define TTAG_ID_MASK                0xFFFFFF00
#define TTAG_VERSION_MASK           0x000000FF

#define TTAG_RESET_OFFSET           0x0	/**< Write to reset */

typedef struct {
  unsigned int rsec0, rnano0, rsec1, rnano1 ; /**< @brief Raw Timestamps */
  int sec0, nano0, sec1, nano1 ; /**< @brief corrected time stamps */
  int idx ;
  int raw100 ;
  int calib100 ;
  int edges ;
  int cnt0, cnt1, prevcnt ;
  int log0, log1 ;
  int duration ;
  int evtnb ;
  int error ;
} fast_status_t ;

#endif
