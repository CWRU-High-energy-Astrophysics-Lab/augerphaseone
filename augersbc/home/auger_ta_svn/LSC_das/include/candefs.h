#if !defined(_CANDEFS_H_)
#define _CANDEFS_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-10-17 09:33:57 +0200 (Mon, 17 Oct 2011) $
  $Revision: 1598 $


  History:
   2010-05-25: Adapted to new CAN ID definitions (see AN-RDA-CAN-App/pdf
               dated 2010-05-15)
   2010-07-08: Adapted (again) see AN-RDA-CAN-App.pdf dated 2010-07-07

********************************************/
/**
 * @defgroup candefs_h CanBus Definitions
 * @ingroup services_include
 *
 */
/**@{*/
/**
 * @file   candefs.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Tue Feb 15 11:10:19 2011
 * 
 * @brief  CanBus Definitions
 * 
 * 
 */

/*
  All data coming from the LR are LITTLE ENDIAN
*/

#define CAN_DEVICE "/dev/can0"

/*
  Odd ID's are from LR to LSC
  Even are from LSC to LR

*/
typedef enum {
  POWER_STATUS_SEND = 0, /* 1st byte: source, 2nd 0x00 = Imminent failure, 0xff = Normal */
  POWER_STATUS_STATE, /* 0x01 - idem */
  CAN_BUFF_STAT_REPLY, /* 0x02 */
  CAN_BUFF_STAT_STATE, /* 0x03 */
  DOWN_STREAM_ACK,       /**< 0x04 - Ack from LS after data stream recevd */
  UP_STREAM_ACK,     /**< 0x05 - Ack from LR ... */
  CHMODE_DONE_UP, /* 0x06 */
  CHMODE_DONE_DOWN, /* 0x07 */
  CHMODE_ACK_UP,  /* 0x08 */
  CHMODE_ACK_DOWN,   /* 0x09 */
  CHMODE_CMD_UP,  /* 0x0a */
  CHMODE_CMD_DOWN, /* 0x0b */
  RESET_HW_CMD_UP,    /* 0x0c */
  RADIO_TRANS_NACK, /* 0x0d */
  NOOP_UP,  /* 0x0e */
  NOOP_DOWN, /* 0x0f */
  DATA_COMMAND_UP_BEGIN = 0x10,	/**< @brief To CDAS: Packet ID increment by 2 */
  DATA_COMMAND_UP_MAX = 0x56,
  DATA_COMMAND_UP_END = 0x58, /* Nb of bytes sent (or 0 to cancel msg) */
  DATA_COMMAND_DOWN_BEGIN = 0x61, /**< @brief From Cdas: Packet ID increment by 2 */
  DATA_COMMAND_DOWN_MAX = 0x83,
  DATA_COMMAND_DOWN_END = 0x85, /* Nb of bytes sent (or 0 to cancel msg) */
  /* */
  MAINT_COMMAND_DOWN_BEGIN = 0x91,
  MAINT_COMMAND_DOWN_MAX = 0xB3,
  MAINT_COMMAND_DOWN_END = 0xB5,
  MAINT_COMMAND_UP_BEGIN = 0x90,
  MAINT_COMMAND_UP_MAX = 0xB2,
  MAINT_COMMAND_UP_END = 0xB4,
  WIRELESS_NET_STATUS = 0xC1,
  WIRELESS_NET_REQ = 0xC2,
  CAN_BUFF_STAT_REQ_DEST = 0xC4,
  CAN_BUFF_STAT_REQ = 0xC5,
  SW_VERSION_REPLY = 0xC6,
  SW_VERSION_REPLY_SOURCE = 0xC7,
  SW_VERSION_REQ_DEST = 0xC8,
  SW_VERSION_REQ = 0xC9,
  HW_SERIAL_REPLY = 0xCA,
  HW_SERIAL_REPLY_SOURCE = 0xCB,
  HW_SERIAL_REQ_DEST = 0xCC,
  HW_SERIAL_REQ = 0xCD,
  ROUTINE_MONITOR_DATA_CGD = 0xCE,
  ROUTINE_MONITOR_DATA_LR = 0xCF,
  GPS_1PPS_STATUS = 0xD0,
  GPS_1PPS_STATUS_REQ = 0xD1,

  GPS_POSITION_REPLY_1 = 0xD2,	/**< @brief Position northing + easting */
  GPS_POSITION_REPLY_2 = 0xD4,	/**< @brief Position altitude + validity */

  GPS_POSITION_REQUEST = 0xD5,
  GPS_DATE_TIME_REPLY = 0xD6,
  GPS_DATE_TIME_REQ = 0xD7,
  ECHO_REPLY_DEST = 0xD8, /* LS answer to ECHO_REQ from LR */
  ECHO_REPLY = 0xD9, /* LR answer to ECHO_REQ_DEST issued by LS */
  ECHO_REQ_DEST = 0xDA, /* Echo request sent by LS to LR */
  ECHO_REQ = 0xDB, /* Echo request received from LR */

  TPCB_FIRST_ID = 0x100,	/**< @brief TPCB Specific */
  TPCB_POWER_MEASURE = TPCB_FIRST_ID,
  TPCB_PRESSURE_TEMP_MEASURE = 0x101,
  TPCB_STARTUP = 0x106,

  LEF_FIRST_ID = 0x200, /**< @brief LED Flasher Specific */
  LEF_SET_MAX = LEF_FIRST_ID,
  LEF_SET_MAX_ACK,
  LEF_SET_VEXT = 0x206,
  LEF_SET_VEXT_ACK,
  LEF_SET_AD = 0x208,		/**< @brief AD5316 NOT used */
  LEF_SET_AD_ACK,
  LEF_GET_ADC = 0x210,		/**< @brief AD5316 NOT used */
  LEF_GET_ADC_ACK,
  LEF_EN_PPS = 0x212,
  LEF_EN_PPS_ACK,
  LEF_TRIG_ONE = 0x214,
  LEF_TRIG_ONE_ACK,
  LEF_ECHO = 0x220,
  LEF_ECHO_ACK,
  LEF_TRIG_MULTI = 0x224,
  LEF_TRIG_MULTI_ACK,
  LEF_WDG_RESET = 0x241,		/**< @brief the LEF restes automgically every ?~2 minutes. Don't care */

  LAST_CANBUS_ID
} CANBUS_ID ;

/*
  Source Identifiers
  LS = 0x20
  LP = 0x10 (Local Power)
  LR = 0x0z (ou z est un sous ensemble de la radio, comme ci-dessous)
   LR_CPU = 0x04
   LR_IOP = 0x02
   LR_CGD = 0x01
*/
#define SOURCE_DEST_LS 0x20
#define SOURCE_DEST_LP 0x10
#define SOURCE_DEST_LR_CPU 0x04
#define SOURCE_DEST_LR_IOP 0x02
#define SOURCE_DEST_LR_CGD 0x01

/*
  Operational Modes
*/
#define NORMAL_OP_MODE 0x05
#define HW_RESET_OP_MODE 0x00
#define SW_RESET_OP_MODE 0x03
#define TEST_STREAM_OP_MODE 0x30

/*
  Wireless Status
    0x00 = Listening mode. Attempt to synchronize.
    0x01 = Synchronized. NIN mode following installation. Attempt to establish net Locus (???)
    0x02 = Synchronized. Net locus established. Ready to begin normal operation
    0x03 = Synchronixed. Net locus established. Normal operation in progress.
*/
#define RADIO_WIRELESS_LISTEN 0x00
#define RADIO_WIRELESS_SYNCH_1 0x01
#define RADIO_WIRELESS_SYNCH_2 0x02
#define RADIO_WIRELESS_NORMAL 0x03
/*
  Software Version
   0: minute
   1: hour
   2: day
   3: month (1-12)
   4: year (2 last digits)
   5: year (2 first digits)
   6: Hw or Sw type
   7: Source
*/
/*
  Position reply
   reply_1: 8 bytes - Northing + easting (little endian)
   reply_2: 5 bytes - Altitude (4 bytes) + validity (0x00 = bad, 0xff = OK)
*/
/*
  GPS date time reply
   UTC time: 4 bytes - (little endian)
   Validity: 1 byte (0x00 = bad, 0xff = OK)
*/

#define CAN_FRAME_LENGTH 10
#define CAN_PAYLOAD_LENGTH 8
#define CAN_PADDING_BYTE 0x55

/* Maximum Nb of bytes UP (from LS to CDAS) */
#define CAN_MAX_UP_BYTES 288

/* Maximum size of data received from Radio */
#define RADIO_FRAME_LENGTH 0x800

/* Monitoring Data */
#define CGD_MONITORING_NUMBER 2
#define LR_MONITORING_NUMBER 6

#define STREAM_ACK_MSG_LENGTH 7
#define STREAM_OK 0xFF
#define STREAM_BAD 0x00

typedef struct {
  unsigned short id_to_ack ;
  unsigned short id_received ;
  unsigned short length ;
  unsigned char status ;
} STREAM_ACK_MSG ;

#define GPS_POSITION_REPLY_1_MSG_LENGTH 8
typedef struct {
  unsigned int northing ;
  unsigned int easting ;
} GPS_POSITION_REPLY_1_MSG ;

#define GPS_POSITION_REPLY_2_MSG_LENGTH 5
typedef struct {
  unsigned int altitude ;
  unsigned char valid ;
} GPS_POSITION_REPLY_2_MSG ;

#define GPS_DATE_REPLY_MSG_LENGTH 5
typedef struct {
  unsigned int date ;
  unsigned char valid ;
} GPS_DATE_REPLY_MSG ;

#define PPS_STATUS_MSG_LENGTH 1
typedef struct {
  unsigned char valid ;
} PPS_STATUS_REPLY_MSG ;

#define POWER_STATUS_SEND_MSG_LENGTH 2
typedef struct {
  unsigned char source ;
  unsigned char valid ;
}  POWER_STATUS_SEND_MSG ;

#define CAN_BUFF_STAT_REPLY_MSG_LENGTH 2
typedef struct {
  unsigned char source ;
  unsigned char nmsg ;
} CAN_BUFF_STAT_REPLY_MSG ;

#define SVR_VERSION_REPLY_MSG_LENGTH 8
typedef struct {
  unsigned int version ;
  unsigned char dummy[3] ;
  unsigned char source ;
} SVR_VERSION_REPLY_MSG ;

typedef struct {
  unsigned char source ;
  unsigned char echo[7] ;
} ECHO_MSG ;

/**@}*/

#endif
