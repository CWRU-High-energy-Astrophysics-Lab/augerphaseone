#if !defined(_CENTRAL_LOCAL_H_)

#define _CENTRAL_LOCAL_H_

/*
  $Author: guglielmi $
  $Date: 2011-09-06 13:43:39 +0200 (Tue, 06 Sep 2011) $
  $Revision: 1491 $

*/

/**
 * @defgroup central_local_h LS/CDAS Message Definitions 
 * @ingroup services_include
 *
 */

/**@{*/

/**
 * @file   central_local.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   8/07/2009
 * 
 * @brief  Definitions for LS - CDAS messages (IDs and structures)
 * 
 * 
 */

/* Completion definition */
#define COMPLETION_ALL    0x00
#define COMPLETION_FIRST  0x40
#define COMPLETION_NEXT   0x80
#define COMPLETION_LAST   0xC0
#define COMPLETION_MASK   0xC0
#define SLICE_MASK        0x3F

/* Destination definition */
#define SINGLE_DESTINATION 0x0
#define BROADCAST_DESTINATION 0x3
#define LIST_DESTINATION 0x1
#define ANTILIST_DESTINATION 0x2
#define DESTINATION_MASK 0xC000
#define DESTINATION_SHIFT 14

typedef enum {			/**< T3 Evts Error Codes */
  NO_ERROR, M_T3_LOST, M_T3_NOT_FOUND, M_T3_TOO_YOUNG, M_T3_ALREADY,
  M_T3_T1_DOWN,
  M_CDAS_BAD_COMPRESS,
  M_CDAS_NO_DATA,
  M_T3_NOSPACE,
  M_T3_UNKNOWN
} ErrorCode;

typedef enum{         /**< MsgSvrIn Error codes */
  MSGIN_LEN, MSGIN_TYP, MSGIN_ALR, MSGIN_LOST
} MsgError;

typedef enum{           /* CalMon Error codes      */
  BUFFER_OK, NO_MONIT_BUFFER, NO_CALIB_BUFFER
} CalMonError;

/**
 * @enum MsgTypeOut
 * @brief Acq/Services to CDAS message types
 *
 **/

typedef enum {
  M_READY,              /**< @brief from Control to CDAS      */
  M_RUN_START_ACK,      /**< @brief      Control to CDAS      */
  M_RUN_PAUSE_ACK,      /**< @brief      Control to CDAS      */
  M_RUN_CONTINUE_ACK,   /**< @brief      Control to CDAS      */
  M_RUN_STOP_ACK,       /**< @brief     Control to CDAS      */
  M_T2_YES,             /**< @brief 5 -  Trigger2 to CDAS      */
  M_T3_EVT,             /**< @brief      EvtSvr to CDAS      */
  M_T3_MUONS,           /**< @brief      EvtSvr to CDAS      */
  M_CONFIG_SET_ACK,     /**< @brief      Control to CDAS      */
  M_MONIT_REQ_ACK,      /**< @brief      CalMon to CDAS      */
  M_MONIT_SEND,         /**< @brief 10  -  CalMon to CDAS      */
  M_CALIB_REQ_ACK,      /**< @brief     CalMon to CDAS      */
  M_CALIB_SEND,         /**< @brief     CalMon to CDAS      */
  M_BAD_SEQUENCE,       /**< @brief      Control to CDAS      */
  M_BAD_VERSION,        /**< @brief      Controly CDAS      */
  M_MSG_ERROR,          /**< @brief  15 -  MsgSvr to CDAS      */
  M_DOWNLOAD_ACK,       /**< @brief      Control to CDAS      */
  M_SHELL_CMD_ACK,      /**< @brief      Control to CDAS      */
  M_MODULE2FLASH_ACK,   /**< @brief      Control to CDAS      */
  M_LOG_SEND,           /**< @brief      Control to CDAS      */
  M_UPLOAD_SEND = M_LOG_SEND,
  M_SET_PARAM_ACK,      /**< @brief  20 -  Control to CDAS      */
  M_UNKNOWN,            /**< @brief      Control to CDAS      */
  M_GENERIC_STR = M_UNKNOWN,        /**< @brief  ALL to CDAS.
				  Generic Warning/Error/Whatever string */
  M_LAST_CENTRAL        /**< @brief  Must be the LAST item */
} MsgTypeOut;

/**
 * @enum MsgTypeIn
 * @brief CDAS to Acq/Services message types
 *
 **/
typedef enum {
  M_REBOOT,             /**< @brief from CDAS to Msgsvr */
  M_WAKEUP,             /**< @brief      CDAS to Msgsvr */
  M_RUN_ENABLE,         /**< @brief      CDAS to  Control */
  M_RUN_START_REQ,      /**< @brief      CDAS to  Control */
  M_RUN_PAUSE_REQ,      /**< @brief      CDAS to Control */
  M_RUN_CONTINUE_REQ,   /**< @brief      CDAS to Control - 5 */
  M_RUN_STOP_REQ,       /**< @brief      CDAS to  Control */
  M_T3_YES,             /**< @brief      CDAS to  EvtSvr  */
  M_CONFIG_SET,         /**< @brief      CDAS to Control */
  M_FLASH_TO_CONFIG,    /**< @brief      CDAS to Control */
  M_CONFIG_TO_FLASH,    /**< @brief      CDAS to Control - 10 */
  M_MONIT_REQ,          /**< @brief      CDAS to CalMon  */
  M_CALIB_REQ,          /**< @brief      CDAS to CalMon  */
  M_DOWNLOAD,           /**< @brief      CDAS to  Download - 0xd (13)*/
  M_DOWNLOAD_CHECK,     /**< @brief      CDAS to Download - 0xe*/
  M_SHELL_CMD,          /**< @brief      CDAS to Shellcmd - 15 */
  M_MODULE2FLASH,       /**< @brief      CDAS to Control */
  M_LOG_REQ,            /**< @brief      CDAS to  Upload */
  M_UPLOAD_REQ = M_LOG_REQ,
  M_SET_PARAM,          /**< @brief      CDAS to Control */
  M_GPS,                /**< @brief      CDAS to GPS     */
  M_MSG_ERROR_IN,        /**< @brief   MsgSvr to Control - 20 */
  M_NO_INPUT,            /**< @brief No msg from CDAS expected */
  M_LAST_LOCAL          /**< @brief  Must be the LAST *****/
} MsgTypeIn;

/* Message to send to CDAS. The message type is NOT part of the payload
   as it is added by the msg server at the right place (possibly in several
   slices)
*/

#define MESSAGE_HEADER_LENGTH 5

/**
 * @struct CDAS_MESSAGE_HEADER
 * @brief Generic Header of a message to CDAS
 *
 */
typedef struct {
  short length ;
  unsigned char slice ;
  unsigned char type ;
  unsigned char msg_nb ;
} CDAS_MESSAGE_HEADER ;

typedef struct {
  int type ;
} TO_CDAS_MESSAGE_HEADER ;

typedef struct {
  TO_CDAS_MESSAGE_HEADER header ;
  unsigned char payload[MAX_MSGSVR_PAYLOAD] ;
} TO_CDAS_MESSAGE ;

/*
  To Cdas Pkts Size = 2400 bits = 300 bytes
  To Cdas Pkt payload size = 300 bytes - header size = 294
*/
#define TO_CDAS_PKT_SIZE 288
#define TO_CDAS_PKT_PAYLOAD_SIZE 281
/* The header does not include the nb of msg */
#define TO_CDAS_PKT_HEADER_LENGTH 7

typedef struct {
  short length ;
  unsigned char pkt_nb ;
  unsigned char reserved ;
  unsigned short lsid ;
  char nb_msg ;
} TO_CDAS_PKT_HEADER ;

typedef struct {
  short length ;
  unsigned char pkt_nb ;
  unsigned char reserved ;
  unsigned short lsid ;
  char nb_msg ;
  unsigned char payload[1] ;
} TO_CDAS_PKT ;

/* The maximum length of a cdas pkt is 16 Kbits ==> 0x4000 */
#define FROM_CDAS_PKT_LENGTH 0x4000
#define FROM_CDAS_PKT_HEADER_LENGTH 4
#define FROM_CDAS_PKT_PAYLOAD_LENGTH (FROM_CDAS_PKT_LENGTH-FROM_CDAS_PKT_HEADER_LENGTH)

typedef struct {
  short length ;
  short rsvrd ;
  unsigned char payload[FROM_CDAS_PKT_PAYLOAD_LENGTH] ;
} FROM_CDAS_PKT ;

#define MREADY_POWERON 0x1
#define MREADT_REBOOT  0x0
#define MREADY_RESTART 0x2
#define MREADY_STARTUP 0x0
#define MREADY_WIRELESS_WAS_BAD 0x4
#define MREADY_WIRELESS_WAS_OK 0x0

/**
 * @struct M_READY_MESSAGE
 * @brief M_READY Message
 *
 * The run_status entry is made of 4 bits
 *   - D0: PowerOn/Reset (currently always set to 0, maybe later ...)
 *   - D1: 0 = Reboot, 1 = Restarted
 *   - D2: 0 = Wireless was and is OK, 1 = Wireless was bad, now OK
 *   - D3: 0 = 1PPS OK, 1 = 1PPS not OK (but wireless is OK, otherwise no
 *         M_READY sent.
 *
 */
typedef struct {
  unsigned char run_status ;
  unsigned char poweron_reset ;
  short utc_offset ;
  unsigned int software_version ;
  unsigned int config_version ;
  int  northing, easting, height ;
  int cur_time ;
  //int utc_offset ;
} M_READY_MESSAGE;

/* This message is the FIRST download message */
typedef struct {
  unsigned short down_nb ;
  unsigned short slice_nb ;	/**< 0xFFFF for first download request */
  unsigned short slice_expected ; /**< Nb of slices */
  unsigned short slice_length ;	/**< Nb of bytes max per slice */
  unsigned short file_name_length ; /**< Length of the file name */
  char fname[2] ;
} M_DOWNLOAD_MESSAGE ;

/*
   For each slice the message is shorter, only down_nb and slice_nb, followed
   by the number of bytes of the slice
*/
typedef struct {
  unsigned short down_nb ;
  unsigned short slice_nb ;	/**< 0xFFFF for first download request */
} M_DOWNLOAD_SLICE_MESSAGE ;

typedef struct {
  unsigned short down_nb ;
  unsigned short check_nb ;	/**< Check Number (??) */
  unsigned short nb_slice ; /**< Nb of slices */
} M_DOWNLOAD_CHECK_MESSAGE ;


typedef struct {
  int first ;
  int number ;
} M_UPLOAD_MESSAGE_HEADER ;

typedef struct {
  M_UPLOAD_MESSAGE_HEADER header ;
  char file_name[32] ;
} M_UPLOAD_MESSAGE ;

typedef struct {
  int size ;
  unsigned char buf[2] ;
} M_UPLOAD_ACQ_MESSAGE ;

typedef struct {
  int cdas_type ;
  unsigned int code ;
} M_SHELLCMD_ACK_MSG ;

/**@}*/

#endif
