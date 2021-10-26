#if !defined(_LOGFILELIB_H_)

#define _LOGFILELIB_H_ 

#include <stdio.h>
/**
 * @defgroup logfile_h Log Files Definitiond
 * @ingroup services_include
 */

/**@{*/

#define ERR_LOGFILE    "error.log"
#define PREV_ERR_LOGFILE "prev_error.log"

#if defined(FOR_LSC)
#define MAX_LOG_SIZE 0x100000
#define MIN_LOG_SIZE 0x1000
#else
#define MAX_LOG_SIZE 0x800000
#define MIN_LOG_SIZE 0x8000
#endif

#define MAXERRORS   20000
#define MAXWARNINGS 40000

enum {
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_FATAL
} ;

#include "logfile_p.h"

/**@}*/

#endif
