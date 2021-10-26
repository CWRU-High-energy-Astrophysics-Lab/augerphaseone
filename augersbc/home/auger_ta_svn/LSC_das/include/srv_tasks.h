#if !defined(_SRV_TASKS_H_)
#define _SRV_TASKS_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-13 17:11:15 #$
  $Revision:: 951              $

********************************************/


/**
 * @defgroup srv_tasks_h Insert something here
 * @ingroup services_include
 */
/**@{*/

/**
 * @file   srv_tasks.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Mon Apr 11 16:04:29 CEST 2011
 * 
 * @brief  Insert brief description here
*/

#include "util.h"


TASK_ENTRY Tasks[] = {
  {"msgsvr", 0, DO_START, HAS_KEY, 500000},
  {"gpsctrl", 0, DO_START, HAS_KEY, 100000},
  {"shellcmd", 0, DO_START, HAS_KEY, 100000},
  {"download", 0, DO_START, HAS_KEY, 100000},
  {"upload", 0, DO_START, HAS_KEY, 100000},
  {"tpcb", 0, DO_START, HAS_NO_KEY, 100000},
  {"ppsirq", 0, DONOT_START, HAS_KEY, 0},
  {NULL, 0, DONOT_START, HAS_NO_KEY, 0}
} ;

#endif
