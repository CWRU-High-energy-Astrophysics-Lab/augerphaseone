#if !defined(_DOWNLOAD_H_)
#define _DOWNLOAD_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-02-14 16:35:37 #$
  $Revision:: 681              $

********************************************/


/**
 * @defgroup download_h Insert something here
 * @ingroup services_include
 */
/**@{*/

/**
 * @file   download.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Mon Feb 14 11:41:56 CET 2011
 * 
 * @brief  Insert brief description here
*/

typedef enum {
  DOWNLOAD_IDLE, DOWNLOAD_RUNNING
} DOWNLOAD_STATE ;

#define MAX_FILE_NAME 256
/* 2 shorts before the slice data */
#define M_DOWNLOAD_HEADER_LENGTH 4
#define SLICES_PER_CHECK_POINT 64
#define SLICE_NB_MASK 0xFFC0
#define DOWN_MAX_SLICE_LENGTH 1024

#define ACK_HEADER_LENGTH 4

#endif
