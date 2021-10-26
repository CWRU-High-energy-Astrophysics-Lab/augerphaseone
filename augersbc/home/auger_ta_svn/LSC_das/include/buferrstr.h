#if !defined(_BMERRSTR_H_)
#define _BMERRSTR_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-26 14:16:31 #$
  $Revision:: 1042             $

********************************************/
/**
 * @defgroup buferrstr_h Buffer Management Library  Errot Strings
 * @ingroup buflib_h
 *
 *
 */
/**@{*/

/**
 * @file   buferrstr.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Fri Feb 11 14:37:21 CET 2011
 * 
 * @brief   Buffer Management Library Error Strings
 *
*/

static char *BufErrStr[] = {
  "Can't create BUF Shared Memory",
  "Can't link to BUF Shared Memory",
  "Bad Ident",
  "Request Not Found",
  "Request Too Old",
  "Request Too Young",
  "Request Already Sent",
  "Search OFF Limits",
  "BUF Unknown Error",
  NULL
} ;

/**@}*/

#endif
