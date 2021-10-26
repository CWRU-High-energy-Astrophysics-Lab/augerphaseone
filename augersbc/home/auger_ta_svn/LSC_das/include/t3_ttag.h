#if !defined(_T3_TTAG_H_)
#define _T3_TTAG_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-26 14:16:31 #$
  $Revision:: 1042             $

********************************************/
/**
 * @defgroup t3_ttag_h  Specific T3 Ttag data definitions
 * @ingroup acq_include
 *
 */

/**@{*/
/**
 * @file   t3_ttag.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Feb  9 15:30:31 2011
 * 
 * @brief  Specific T3 Ttag data definitions
 * 
 * 
 */


typedef struct {
  unsigned int cur_100,		/**< @brief calib 100 of second */
    next_100,			/**< @brief calib 100 of second+1 */
    pprev_saw,			/**< @brief Sawtooth of second-2 */
    prev_saw,			/**< @brief Sawtooth of second-1 */
    cur_saw,			/**< @brief Sawtooth of second */
    rcv_offset ;		/**< Receiver offset (from gpsconfig) */
} TTAG_BLOCK ;

/**@}*/

#endif
