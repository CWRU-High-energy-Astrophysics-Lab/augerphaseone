#if !defined(_DAS_TASK_H_)
#define _DAS_TASK_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-13 17:18:57 #$
  $Revision:: 961              $

********************************************/

#include "util.h"

TASK_ENTRY Tasks[] = {
  {"control", 0, DO_START, HAS_KEY, 1000000},
  {"calmonsvr", 0, DO_START, HAS_KEY, 200000},
  {"monitor", 0, DO_START, HAS_KEY, 0},
  {"trigger2", 0, DO_START, HAS_KEY, 100000},
  {"evtsvr", 0, DO_START, HAS_KEY, 0},
  {"t1read", 0, DO_START, HAS_NO_KEY, 100000},
  {"t1irq", 0, DO_START, HAS_NO_KEY, 0},
  {"t1fake", 0, DO_START, HAS_KEY, 0 },
  {"mufill", 0, DO_START, HAS_KEY, 0},
  {"muread", 0, DO_START, HAS_NO_KEY, 10000},
  {"muirq", 0, DO_START, HAS_NO_KEY, 0 },
  {"mufake", 0, DO_START, HAS_NO_KEY, 0 },
  {"grbread", 0, DO_START, HAS_NO_KEY, 0},
  {NULL, 0, DONOT_START, HAS_NO_KEY, 0}
} ;

#endif
