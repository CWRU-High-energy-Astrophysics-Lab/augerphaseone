#if !defined(_T3_ERR_H_)
#define _T3_ERR_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2010-11-24 14:18:50 #$
  $Revision:: 286              $

********************************************/

#include "central_local.h"

static char * T3Errors[] = {
  "No Error", "T3 Too Old", "T3 Not Found", "T3 Too Young",
  "T3 Already", "No T1", "T3 Bad Compress", "T3 No Data", "T3 No Space",
  "Unknown Error",
  NULL
} ;

#endif
