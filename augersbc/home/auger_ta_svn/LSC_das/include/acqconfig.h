#if !defined(_ACQCONFIG_H_)
#define _ACQCONFIG_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-22 13:12:06 #$
  $Revision:: 1796             $

********************************************/

/**
 * @defgroup acqconfig_h  Acquisition Configuration definitions
 * @ingroup acq_include
 *
 * Contains the definition of the acqconfig structure.
 * The global structure is made of
 *  - initialized: set when loading the file into the acqconfig shared mem
 *  - num_version: Version Number
 *  - fe_params: the front end parameters FE_PARAMS structure
 *  - acq_params: the acqusition parameters ACQ_PARAMS structure
 *  - monit_params: the monitoring parameters MONIT_PARAMS structure
 *
 */

/**@{*/
/**
 * @file   acqconfig.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Feb  9 15:30:31 2011
 * 
 * @brief  Acquisition Configuration definitions
 * 
 * 
 */

#define ACQ_CONFIG_FILE_NAME "AcqConfig.cfg"
#define ACQ_CONFIG_FILE_DUMP "AcqConfig.dmp"
#define ACQ_CONFIG_NAME "AcqConfig"

/* In the current version of the LSC, the zero of the FADC is set at 512 ADU */
#define ADC_BASE_LINE 512

#define MONIT_AVERAGE_PERIOD 300
#define MONIT_SCAN_PERIOD 5 ;
#define MONIT_SEND_PERIOD 300
/* Default High Voltage: 0V ! */
#define HV_PMT 0

typedef struct {
  int scan_rate ;		/**< Frequency to read ADC's (seconds) */
  int avg_rate ;		/**< Nb of read before averaging and write
				 to monit buffer */
  int send_rate ;		/**< Frequency to send monit to CDAS */
  unsigned int hv_pmt ;		/**< Default  PMT High Voltage (Volts) */
} MONIT_PARAMS ;

/**
 * @enum T2_ALGO_TYPES
 * 
 * - T2_STANDARD_ALGO: compute qualiy of T1.
 * - T2_LED_ALGO: keep only LED triggers
 * - T2_RANDOM_ALGO: generate 20% randomly (used for fakes T1 ?)
 * - T2_ALL_ALGO: keep all T1's as T2 up to MAX_T2_RATE
 *
*/
typedef enum {
  T2_STANDARD_ALGO = 1, T2_LED_ALGO, T2_RANDOM_ALGO, T2_ALL_ALGO
} T2_ALGO_TYPES ;

#define T2_DEFAULT_ALGO T2_STANDARD_ALGO

#define MAX_T1_MISS 200
#define MAX_T1_RATE 200
#define MAX_T2_RATE 40
#define VEM_MIN 50

/* ToT A is defined as a threshold trigger */
#define TOT_A_WIDTH 120
#define TOT_A_THRESH 768
#define TOT_A_OCCUP 1

/* ToT B is really a time over threshold trigger */
#define TOT_B_WIDTH 120
#define TOT_B_THRESH 650
#define TOT_B_OCCUP 32

/* ToT D */
#define TOT_D_WIDTH 120
#define TOT_D_OCCUP 32
#define TOT_D_LOWER ADC_BASE_LINE
#define TOT_D_UPPER ADC_BASE_LINE+100
#define TOT_D_FD 0
#define TOT_D_FN 0

/* Software trigger time delay (10 micros) */
#define SOFT_DELAY 100

/* Muon */
#define MUON_THRESH 600

/**
 * @struct TOT_AB_PARAMS
 * @brief Time Over Threshold A and B parameters.
 *
 * The parameters are taken
 * from the acqconfig shared memory.
 *
 * 
 */

typedef struct {
  unsigned int width ;		/**< @brief Width in ADC Bins */
  unsigned int thresh ;		/**< @brief Threshold in ADU */
  unsigned int occupancy ;	/**< @brief Occupancy in ADC Bins */
  unsigned int enable ;		/**< @brief 1 if enable, 0 otherwise */
} TOT_AB_PARAMS ;

/**
 * @struct TOT_D_PARAMS
 * @brief Time Over Threshold Deconvoluted parameters.
 *
 * The parameters are taken
 * from the acqconfig shared memory.
 *
 * 
 */
typedef struct {
  unsigned int lower ;
  unsigned int upper ;
  unsigned int occupancy ;
  unsigned int width ;
  unsigned int FD ;
  unsigned int FN ;
  unsigned int enable ;
} TOT_D_PARAMS ;


/**
 * @struct MUON_PARAMS
 * @brief Muon Trigger Parameters. To be Implemented.
 *
 */
typedef struct {
  int thresh ;			/**< @brief Muon Threshold */
  int enable ;		/**< @brief Enable/Disable muon triggers */
} MUON_PARAMS ;

/**
 * @struct SOFT_PARAMS
 * @brief Software Trigger parameters.
 *
 * The parameters are taken
 * from the acqconfig shared memory.
 *
 * 
 */
typedef struct {
  unsigned int delay ;		/**< @brief delay in decananos */
  unsigned int enable ;
} SOFT_PARAMS ;

/**
 * @struct EXT_PARAMS
 * @brief External Trigger parameter.
 *
 */
typedef struct {
  unsigned int enable ;
} EXT_PARAMS ;

typedef struct {
  TOT_AB_PARAMS tot_A ;		/**< @brief Time Over Threshold A trigger */
  TOT_AB_PARAMS tot_B ;		/**< @brief Time Over Threshold B trigger */
  TOT_D_PARAMS tot_D ;		/**< @brief Time Over Threshold D(econvoluted) trigger */
  SOFT_PARAMS soft ;		/**< @brief Software Trigger */
  /* External Trigger */
  EXT_PARAMS external ;		/**< @brief External Trigger */
  /* Muon Trigger - TBD */
  MUON_PARAMS muon ;		/**< @brief Muon Trigger */
  int grb_enable ;		/**< @brief 1 to Enable Grb */
  unsigned int reserved[16] ;	/**< Reserved for future use (16 words) */
} FE_PARAMS ;

typedef struct {
  unsigned int vem_min ;
  int t2_algo ;
  int max_t1_miss ;
  int max_t1_rate ;
  int max_t2_rate ;
} ACQ_PARAMS ;

/**
 * @struct ACQ_CONFIG
 * @brief Global Config Structure
 *
 * The global structure is made of
 *  - initialized: set when loading the file into the acqconfig shared mem
 *  - num_version: Version Number
 *  - fe_params: the front end parameters FE_PARAMS structure
 *  - acq_params: the acqusition parameters ACQ_PARAMS structure
 *  - monit_params: the monitoring parameters MONIT_PARAMS structure
 * 
 */

typedef struct {
  unsigned int initialized ;
  unsigned int num_version ;
  FE_PARAMS fe_params ;
  ACQ_PARAMS acq_params ;
  MONIT_PARAMS monit_params ;
  unsigned int reserved[8] ;
} ACQ_CONFIG ;

/**@}*/

#endif
