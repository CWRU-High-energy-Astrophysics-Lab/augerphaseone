#if !defined(_EVENTS_H_)
#define _EVENTS_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-22 13:13:40 #$
  $Revision:: 1798             $

********************************************/

/**
 * @defgroup events_h  Fast and Slow Events definitions
 * @ingroup acq_include
 *
 *  - Fast (T1) Events
 *     - At 100 MHz, 1024 samples = 10.24 micros, should be enough
 *     - With 6 FADC, 10 bits
 *         - 8 bytes (64 bits) per sample
 *         - 8 Kbytes per event
 *     - 100 events per second
 *         - 800 Kbytes per second
 *     - 10 seconds of data stored
 *         - 8 Mega Bytes
 *
 * As we have 128 Meg of Ram, we could easily use 16 Megs for the event buffer
 *  and store ~ 20 seconds of T1 data
 *
 *  - Slow (Muon) Events
 *    - In average 10 Events per second
 *    - 8 Kbytes per event
 *    - 20 seconds of Muons
 *      - Less than 2 MBytes
 */

/**@{*/
/**
 * @file   events.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Feb  9 16:42:17 2011
 * 
 * @brief   Fast and Slow Events definitions
 * 
 * 
 */

/*********************
  Definitions for Fast (aka T1) events
********************/
/*
  At 100 MHz, 1024 samples = 10.24 micros, should be enough
  With 6 FADC, 10 bits ==> 8 bytes (64 bits) per sample
   ==> 8 Kbytes per event
  100 events per second
   ==> 800 Kbytes per second
  10 seconds of data stored
   ==> 8 Mega Bytes

  As we have 128 Meg of Ram, we could easily use 16 Megs for the event buffer
   ==> store ~ 20 seconds of T1 data

*/
#include "fe_defs.h"
#include "timestamp.h"

#define FAST_BUFFER_NAME "FastBuffer"
#define EVENT_BUFFER_NAME "EventBuffer"

#define FAST_SAMPLE_NUMBER FAST_EVT_SAMPLES
#define FAST_EVTS_PER_SECOND 100
#define FAST_EVTS_MAX_TIME 20
#define EVENT_BUFFER_NB_EVENTS  (FAST_EVTS_PER_SECOND*FAST_EVTS_MAX_TIME)

#define FAST_UNKNOWN_TYPE 0xFFFF

/**
 * @struct FAST_SAMPLE
 * @brief A Fast Sample is made of 2 32 bits unsigned int
 * 
 */

typedef struct {
  unsigned int fadc123, fadc456;
} FAST_SAMPLE ;

/**
 * @struct FAST_EVENT
 * @brief Fast Event Structure (aka T1 events, aka /EVTCLKF events)
 * 
 */

typedef struct {
  TIME_STAMP date ;		/**< @brief Leading + trailing edges time */

  int micro_off ;		/**< @brief Offset in micros */
  unsigned short T2T1_type ;	/**< @brief TOT or Threshold or ... */
  unsigned short nsamples ;	/**< @brief Nb of samples. Should be equal to
				 FAST_SAMPLE_NUMBER */
  unsigned int status ;		/**< @brief Status of the shower buffers */
  unsigned int resvrd[7] ;	/**< @brief Reserved for future use */
  FAST_SAMPLE data[FAST_SAMPLE_NUMBER] ; /**< @brief the Data */
} FAST_EVENT ;

/*********************
  Definitions for Slow (aka /EVTCLKS or Muons) events
  Preliminary !!!! To Be Decided !!!
********************/

/**
 * @def MUON_BUFFER_NAME
 * @brief The name of the Muon Buffer Shared memory filled by mufill
 * 
 */
/**
 * @def SLOW_BUFFER_NAME
 * @brief The name of the Slow Buffer Shared memory filled by muread and emptied by mufill
 * 
 */
/**
 * @def MAX_SLOW_EVENTS
 * @brief A maximum of 10 Muons events is kept in the Slow Buffer
 * 
 */

#define MUON_BUFFER_NAME "MuonBuffer"
#define SLOW_BUFFER_NAME "SlowBuffer"
#define MAX_SLOW_EVENTS  10

/**
 * @def MUON_EVTS_PER_SECOND
 * @brief Average nb of Muons Events per second
 *
*/
/**
 * @def MUON_BUFFER_NB_EVENTS
 * @brief Number of Muons Events in the Muon Buffer. About 20 seconds.
 *
*/

#define MUON_EVTS_PER_SECOND 10
#define MUON_BUFFER_NB_EVENTS (MUON_EVTS_PER_SECOND* FAST_EVTS_MAX_TIME)

/**
 * @def MUON_BUFFER_SIZE
 * @brief The maximum size of a muon buffer (in the FPGA) is 8kbytes (0x2000)
 *
 *  A "muon event" IRQ is generated when the muon buffer is full.
 * A muon buffer is made of a maximum of 4096 samples, 4 bytes per sample
*/
#define MUON_BUFFER_SIZE MUON_EVT_SIZE

#if 0
typedef struct {
  unsigned char sum, pmt ;
} MUON_DATA ;

typedef struct {
  unsigned int time_tag ;	/**< @brief Muon Time tag in decananos */
  MUON_DATA sample0, sample1, sample2, sampl3 ;
} MUON_SAMPLE ;
#endif

typedef struct {
  ONE_TIME date ;		/**< @brief Timestamp of the Muon IRQ */
  int bufsize  ;		/**< @brief Actual buffer size */
  unsigned int data[MUON_EVT_SAMPLES] ;	/**< @brief Muon Buffer (set to maximum size) */
} MUON_EVENT ;


/**@}*/

#endif
