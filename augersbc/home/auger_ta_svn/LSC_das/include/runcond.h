#if !defined(_RUNCOND_H_)

#define _RUNCOND_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-09-09 12:36:11 +0200 (Fri, 09 Sep 2011) $
  $Revision: 1505 $

*/

/**
 * @defgroup runcond_h Run Conditions Definitions 
 * @ingroup services_include
 *
 *
 *  Already known Run Conditions:
 *  - FORCE_WIRELESS_COND   : If present forces wireless staus OK (msgsvr).
 *  - FORCE_GPS_COND        : if present forces the Gps Status OK (gpsctrl)
 *  - T2_ALGO_COND          : Set T2 Algo independently of AqcConfig (control).
 *                            T2_ALGO=1,2,3 or 4
 *  - T2_MAX_COND           : Set the Max nb of T2 independently of AqcConfig (control).
 *                            T2_MAX=<nn> where 0 < nn <= 89
 */

/**@{*/

/**
 * @file   runcond.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   9/09/2011
 * 
 * @brief  Definitions for Run Conditions
 *
 *
 *  Already known Run Conditions:
 *  - FORCE_WIRELESS_COND   : If present forces wireless staus OK (msgsvr).
 *  - FORCE_GPS_COND        : if present forces the Gps Status OK (gpsctrl)
 *  - T2_ALGO_COND          : Set T2 Algo independently of AqcConfig (control).
 *                            T2_ALGO=1,2,3 or 4
 *  - T2_MAX_COND           : Set the Max nb of T2 independently of AqcConfig (control).
 *                            T2_MAX=<nn> where 0 < nn <= 89
 *
*/

#define FORCE_WIRELESS_COND "WIRELESS"
#define FORCE_GPS_COND "GPS"
#define T2_ALGO_COND "T2_ALGO"
#define T2_MAX_COND "T2_MAX"

char * IsCond( char * the_cond, int * found ) ;
void ShowCond( FILE * ) ;
void LogCond() ;
int SetCond() ;

#endif

