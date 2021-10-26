#if !defined(_ACQ_STATUS_H_)
#define _ACQ_STATUS_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-22 13:12:56 #$
  $Revision:: 1797             $

********************************************/

/**
 * @defgroup acqstatus_h  Acquisition Status definitions
 * @ingroup acq_include
 *
 */
/**@{*/
/**
 * @file   acqstatus.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Feb  9 16:42:17 2011
 * 
 * @brief  Acquisition Status definitions
 * 
 * 
 */

#define ACQ_STATUS_NAME "acqstatus"

typedef struct {
  unsigned int start_time ;	/**< @brief Triggering start time (GPS time) */
  unsigned int stop_time ;	/**< @brief Triggering stop time (GPS time) */
  int running ;			/**< @brief 1 if triggering, 0 otherwise */
  int TotA_cnt,			/**< @brief Number of ToT A triggers */
    TotB_cnt,			/**< @brief Number of ToT B triggers */
    TotD_cnt,			/**< @brief Number of ToT D triggers */
    Sft_cnt,			/**< @brief Number of Software triggers */
    Ext_cnt,			/**< @brief Number of External trigers */
    Unk_cnt ;			/**< @brief Number of Unknown type triggers */
  int T1_cnt ;			/**< @brief Total T1 count */
  int T2_cnt ;			/**< @brief Number of T2 */
  int T1Miss ;			/**< @brief Number of Missed T1 triggers */
  int T1HighRate ;		/**< @brief set when T1 RATE higher than MAX */
  int MuonMiss ;		/**< @brief Number of missed Muon triggers */
  int Muon_cnt ;		/**< @brief Number of Muon triggers */
  double deadtime ;		/**< @brief Computed dead time */
  double max_deadtime ;		/**< @brief Maximum dead time on 1 T1 */
  int max_buf_full ;		/**< @brief Max nb of buffer full */
  int last_second ;		/**< @brief Time (seconds) of the last T1 */
  int last_nano ;		/**< @brief Time (decananos) of the last T1 */
  int reserved[16] ;
} trigger_status_t ;

#define MAX_GRB_VALUES 32
#define GRB_MASK 0x1F ;

typedef struct {
  unsigned int initialized ;
  unsigned int fpga_version ;	/**< @brief FPGA Trigger Version */
  unsigned int fpga_type ;	/**< @brief Nitz or Courty type */
  unsigned int acq_version ;	/**< @brief Acquisition Version */
  unsigned int acq_start_time ;
  int has_fe ;			/**< @brief 1 if Front End present, 0 otherwise */
  trigger_status_t Trigger_stat ;
  int grb_counter[MAX_GRB_VALUES] ;
} acq_status_t ;

/**@}*/

#endif
