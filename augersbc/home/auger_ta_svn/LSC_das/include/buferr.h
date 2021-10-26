#if !defined(_BUFERR_H_)

#define _BUFERR_H_ 
/**
 * @defgroup buferr_h Buffer Management Library Error Codes
 * @ingroup buflib_h
 *
 *
 */
/**@{*/

/**
 * @file   buferr.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Fri Feb 11 14:37:21 CET 2011
 * 
 * @brief   Buffer Management Library Error Codes
 *
*/

typedef enum {
  BUF_OK = 0,
  BUF_CREATE_ERROR = 0x8000,
  BUF_LINK_ERROR,
  BUF_BAD_IDENT_ERROR,
  BUF_REQUEST_NOT_FOUND,
  BUF_REQUEST_TOO_OLD,   /* 8004 */
  BUF_REQUEST_TOO_YOUNG, /* 8005 */
  BUF_REQUEST_ALREADY,   /* 8006 */
  BUF_OFF_LIMITS,        /* 8007 */
  BUF_ERROR_UNKNOWN
} BUF_ERROR_TYPE ;

/**@}*/

#endif
