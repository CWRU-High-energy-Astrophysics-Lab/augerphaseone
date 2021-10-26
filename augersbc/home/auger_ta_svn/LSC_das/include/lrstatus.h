#if !defined(_LRSTATUS_H_)
#define _LRSTATUS_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-22 12:43:51 #$
  $Revision:: 1779             $

********************************************/


/**
 * @defgroup lrstatus_h Insert something here
 * @ingroup services_include
 */
/**@{*/

/**
 * @file   lrstatus.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Thu Aug 18 14:34:40 EDT 2011
 * 
 * @brief  Routine Monitoring Data from LR and Channel Guardian
 *         description.
*/

#define LR_STATUS_NAME "LrStatus"
/* Nb of Monitoring Data sent by LR/CGD
   Actually 8 CAN frames, 6 data bytes per frame
   But the first item of the first LR frame is on 2 bytes, thus only 47
   different data.
   Some of these data are "reserved for future use"
*/
#define MAX_LR_MONIT_DATA 47

/**
 * @struct LR_STATUS
 * @brief Local Radio Monitoring Data structure
 *
 */
typedef struct {
  unsigned int Initialized ;
  time_t last_update ;
  int data[MAX_LR_MONIT_DATA] ;
} LR_STATUS ;

/**@}*/

#endif
