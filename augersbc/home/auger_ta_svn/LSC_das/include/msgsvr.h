#if !defined(_MSGSVR_H_)
#define _MSGSVR_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-08-01 12:16:21 +0200 (Mon, 01 Aug 2011) $
  $Revision: 1357 $

********************************************/
/**
 * @defgroup msgsvr_h Message Server Definitions
 * @ingroup services_include
 *
 */

/**@{*/

#define MSGSVR_QUEUE_NAME "msgsvr"
#define MSGSVR_STATUS_NAME "svrstatus"

enum {
  MSGSVR_CMD = 1,		/**< @brief Message to the Msg Server */
  TO_RADIO_MSG,			/**< @brief Message to the radio. To send NOW */
  HIGH_PRIORITY, MEDIUM_PRIORITY, LOW_PRIORITY, /**< @brief Messages are for CDAS,
						  merged into one message
						  based on priority */
  MSGSVR_FROM_CDAS,
  SIGNAL_MSG = 100
} ;

#define MAX_MSGSVR_PAYLOAD 0x10000

/*
  The type is one of the type described above
  The length is the actual length of the payload
  The position is the position of this LsId in the List (needed by M_T3_YES)
*/
typedef struct {
  int type ;
  int length ;
  int position ;
  int nb_ls ;
} MSGSVR_PKT_HEADER ;

typedef struct {
  MSGSVR_PKT_HEADER header ;
  unsigned char payload[MAX_MSGSVR_PAYLOAD] ;
} MSGSVR_PKT ;

typedef struct {
  unsigned int signal ;
} MSGSVR_SIGNAL_MSG ;

typedef struct {
  int nb_pkt ;
  int tot_size ;
} MSGSVR_STATUS_IN_OUT ;

enum {
  WIRELESS_STATUS_BAD,
  WIRELESS_STATUS_OK
} ;

typedef struct {
  unsigned int ServicesVersion ;
  unsigned int CpuNumber ;
  MSGSVR_STATUS_IN_OUT from_cdas ; /**< @brief Nb of messages FROM CDAS */
  MSGSVR_STATUS_IN_OUT to_cdas ; /**< @brief Nb of messages TO CDAS */
  MSGSVR_STATUS_IN_OUT from_client ; /**< @brief Nb of messages FROM clients */
  MSGSVR_STATUS_IN_OUT to_client ; /**< @brief Nb of messages TO clients */
  int can_out ;		/**< @brief Time of the last CAN frame sent */
  int can_in ;		/**< @brief Time of the last CAN frame received */
  int can_overr ;		/**< @brief Can Overrun error counter */
  int wireless ;		/**< @brief Status of the Radio network */
} MSGSVR_STATUS ;

/**@}*/

#endif
