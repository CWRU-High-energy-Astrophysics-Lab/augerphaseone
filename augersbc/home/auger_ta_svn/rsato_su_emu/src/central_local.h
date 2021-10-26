/*
   $Author: guglielm $
   $Date: 2007-09-11 09:54:45 $
   $Log: not supported by cvs2svn $
   Revision 1.1.1.1  2003/12/04 21:58:23  bertou
   Initial import of all software

   Revision 1.10  2002/02/15 09:56:43  guglielm
   Added message T3_T1_DOWN (only if EVTSVR_VERSION > 4)

   Revision 1.9  2001/11/13 13:13:46  guglielm
   Added GENERIC msg in enum (=M_UNKNOWN)
   Added UPLOAD (=M_LOG_REQ)
   Added BUFCOMSIZE depending on SU version (149 instead of 150)

   Revision 1.8  2001/06/01 10:59:36  guglielm
   Cosmetics

   Revision 1.7  2001/05/31 15:01:35  jmb

    	central_local.h fakes.h
           Cosmetics

   Revision 1.6  2001/05/28 10:49:06  guglielm
   Added message M_MSG_ERROR_IN (from MsgSvrIn to Control).
   Was M_MSG_ERROR (but part of LS to CS enum)

   Revision 1.5  2001/05/23 10:56:16  jmb

    Modified Files:
    	central_local.h monitor.h run_config.h

   ----------------------------------------------------------------------

   Revision 1.4  2001/04/12 09:38:19  guglielm
   Added Mask for completion

   Revision 1.3  2001/03/30 14:42:19  castera
   Changed messages order to Keep Flavia compatible
    Modified Files:
    	central_local.h
    ----------------------------------------------------------------------

   Revision 1.2  2001/03/29 09:01:51  jmb

   	central_local.h
   	add LAST message  types

   Revision 1.1.1.1  2001/03/28 15:03:07  revenu
   no message

   Revision 1.5  2001/03/01 13:12:38  os9
   debugged version (1/03/01)

*/
/*
  central_local.h : Used by both Central & Local Softwares
  ---------------   Pierre Auger Project

              JM Brunet, L Gugliemi, G Tristram
	           College de France - Paris

   Created  : 10/09/97
   Modified : 29/07/98 : Version 2
            : 16/03/99 : Version 2.2
            : 22/02/00 : Version 2.3
	    : 24/05/00 : Version 2.4
	    : 07/09/00 : Version 2.6
*/


#ifndef _CENTRAL_LOCAL_
#define _CENTRAL_LOCAL_

                        /* Completion definition */
#define COMPLETION_ALL    0x00
#define COMPLETION_FIRST  0x40
#define COMPLETION_NEXT   0x80
#define COMPLETION_LAST   0xC0
#define COMPLETION_MASK   0xC00

typedef enum {
  WAIT_STATE= 0,
  START_RUN_REQUESTED,    RUN_STARTED,
  STOP_RUN_REQUESTED,     RUN_STOPPED,
  PAUSE_RUN_REQUESTED,    RUN_PAUSED,
  CONTINUE_RUN_REQUESTED,
  REBOOT
} run_status;


typedef enum {          /* Event Serveur Error Codes */
  NO_ERROR, M_T3_LOST, M_T3_NOT_FOUND, M_T3_TOO_YOUNG, M_T3_ALREADY
  ,M_T3_T1_DOWN
  ,M_CDAS_BAD_COMPRESS
  ,M_CDAS_NO_DATA
  ,M_T3_NOSPACE
} ErrorCode;

typedef enum{           /* MsgSvrIn Error codes      */
  MSGIN_LEN, MSGIN_TYP, MSGIN_ALR, MSGIN_LOST
} MsgError;

typedef enum{           /* CalMon Error codes      */
  BUFFER_OK, NO_MONIT_BUFFER, NO_CALIB_BUFFER
} CalMonError;

typedef enum {          /* Messages from Local to Central station */
  M_READY,              /* from Control to CS      */
  M_RUN_START_ACK,      /*      Control    CS      */
  M_RUN_PAUSE_ACK,      /*      Control    CS      */
  M_RUN_CONTINUE_ACK,   /*      Control    CS      */
  M_RUN_STOP_ACK,       /*      Control    CS      */
  M_T2_YES,             /*      Trigger2   CS      */
  M_T3_EVT,             /*      EvtSvr     CS      */
  M_T3_MUONS,           /*      EvtSvr     CS      */
  M_CONFIG_SET_ACK,     /*      Control    CS      */
  M_MONIT_REQ_ACK,      /*      CalMon     CS      */
  M_MONIT_SEND,         /*      CalMon     CS      */
  M_CALIB_REQ_ACK,      /*      CalMon     CS      */
  M_CALIB_SEND,         /*      CalMon     CS      */
  M_BAD_SEQUENCE,       /*      Control    CS      */
  M_BAD_VERSION,        /*      Control    CS      */
  M_MSG_ERROR,          /*      MsgSvrIn   CS      */
  M_DOWNLOAD_ACK,       /*      Control to CS      */
  M_OS9_CMD_ACK,        /*      Control to CS      */
  M_MODULE2FLASH_ACK,   /*      Control to CS      */
  M_LOG_SEND,           /*      Control to CS      */
  M_UPLOAD_SEND = M_LOG_SEND,
  M_SET_PARAM_ACK,      /*      Control to CS      */
  M_UNKNOWN,            /*      Control    CS      */
  M_GENERIC_STR = M_UNKNOWN,        /*        all   to CS
				  Generic Warning/Error/Whatever string */
  M_LAST_CENTRAL        /***  Must be the LAST *****/
} MsgTypeOut;


typedef enum {          /* Messages from Central to Local station */
  M_REBOOT,             /* from CS      to Control */
  M_WAKEUP,             /*      CS         Control */
  M_RUN_ENABLE,         /*      CS         Control */
  M_RUN_START_REQ,      /*      CS         Control */
  M_RUN_PAUSE_REQ,      /*      CS         Control */
  M_RUN_CONTINUE_REQ,   /*      CS         Control */
  M_RUN_STOP_REQ,       /*      CS         Control */
  M_T3_YES,             /*      CS         EvtSvr  */
  M_CONFIG_SET,         /*      CS         Control */
  M_FLASH_TO_CONFIG,    /*      CS         Control */
  M_CONFIG_TO_FLASH,    /*      CS         Control */
  M_MONIT_REQ,          /*      CS         CalMon  */
  M_CALIB_REQ,          /*      CS         CalMon  */
  M_DOWNLOAD,           /*      CS         Control */
  M_DOWNLOAD_CHECK,     /*      CS         Control */
  M_OS9_CMD,            /*      CS         Control */
  M_MODULE2FLASH,       /*      CS         Control */
  M_LOG_REQ,            /*      CS         Control */
  M_UPLOAD_REQ = M_LOG_REQ,
  M_SET_PARAM,          /*      CS         Control */
  M_GPS,                /*      CS         GPS     */
  M_MSG_ERROR_IN,        /*   MsgSvrIn      Control */
  M_LAST_LOCAL          /***  Must be the LAST *****/
} MsgTypeIn;

#define BUFCOMSIZE 149
#define PACKETSIZE 265     /* Maximun size of comms packet      */

#endif

