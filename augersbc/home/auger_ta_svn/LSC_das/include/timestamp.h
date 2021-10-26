#if !defined(_TIMESTAMP_H_)
#define _TIMESTAMP_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-02-11 17:20:02 +0100 (Fri, 11 Feb 2011) $
  $Revision: 660 $

********************************************/


/**
 * @defgroup timestamp_h Time Stamps definitions
 * @ingroup services_include
 *
 */

/**@{*/
/**
 * @file   timestamp.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Feb  9 15:30:31 2011
 * 
 * @brief  Time Stamps definitions
 * 
 * 
 */

typedef struct {
  unsigned int second ;
  unsigned int nano ;
} ONE_TIME ;

typedef struct {
  ONE_TIME first ;
  ONE_TIME secnd ;
} TIME_STAMP ;

/**@}*/

#endif
