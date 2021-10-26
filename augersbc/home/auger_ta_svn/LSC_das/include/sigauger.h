#if !defined(_SIGAUGER_H_)
#define _SIGAUGER_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2012-06-14 13:40:21 +0200 (Thu, 14 Jun 2012) $
  $Revision: 1886 $

********************************************/
/* (re)read Config file (after a buildconfig) */
#define SIG_READ_CONFIG 1000

#define SIG_SAVE_CONFIG 1001

/* Signals 3000 to 3999 are supposed to be handled by all processes. */
/* General signals setting the verbosity. */
#define SIG_RESET_VERBOSE 3000
#define SIG_INCREMENT_VERBOSE 3001
#define SIG_DECREMENT_VERBOSE 3002
/* General signals to check potential memory leaks:
   The process should call the function mem_status and report either
    - to log file (SIG_MEMLIB_STATUS_LOG)
    - or to CDAS (SIG_MEMLIB_STATUS_MSG) via a generic message
  For example from CDAS:
   shellcmd: stop -3011 msgsvr
*/
#define SIG_MEMLIB_STATUS_LOG 3010
#define SIG_MEMLIB_STATUS_MSG 3011

/* To msgsvr from gpsctrl */
#define SIG_PPS_IS_GOOD 1100
#define SIG_PPS_IS_BAD 1101
#define SIG_GPS_HAS_DATE   1102
#define SIG_GPS_READY 1103
#define SIG_FORCE_WIRELESS_OK 1104
#define SIG_FORCE_WIRELESS_BAD 1105

/* From ppsirq: 1PPS occurred */
#define PPS_SIGNAL_READY 10000

/* Specific for GpsCtrl */
#define SIG_GPSCTRL_GET_SATELLITES 4000
#define SIG_GPSCTRL_FORCE_OK       4001
#define SIG_GPSCTRL_SEND_MREADY    4002
/* Gpsctrl sends its status as a Generic Message */
#define SIG_GPSCTRL_STATUS_STR     4003
/* Gpsctrl send date and time to radio */
#define SIG_GPSCTRL_SEND_DATE      4004
/* Gpsctrl sets the system time according to UTC time from GPS */
#define SIG_GPSCTRL_SET_DATE       4005
/* Gpsctrl request almanach status */
#define SIG_GPSCTRL_ALMANACH_STATUS 4006
/* Gpsctrl request almanach data (and save to /ram0/almanach.cfg) */
#define SIG_GPSCTRL_ALMANACH_SAVE   4007
/* Gpsctrl send almanach data to receiver from /ram0/almanach.cfg) */
#define SIG_GPSCTRL_ALMANACH_SEND   4008

/*  Specific for for Download */
/* Reset download if a download has been interrupted */
#define SIG_DOWNLOAD_RESET 1000

/* Trigger Signals */
// Start triggering
#define SIG_START_TRIGGER 12345
// Stop triggering
#define SIG_STOP_TRIGGER  12346
// Force usage of t1fake (soft triggers)
#define SIG_FAKE_SOFT_TRIGGERS 1234
// Disable trigger2, replaced by golin
#define SIG_ENABLE_GOLIN  1235
// Enable trigger2, golin no longer in use
#define SIG_DISABLE_GOLIN 1236

/* Fake T1 signals */
#define SIG_FAKE_T1_MIN 5001
#define SIG_FAKE_T1_MAX 5200
#define SIG_FAKE_RANDOM 6001
#define SIG_FAKE_NO_RANDOM 6000
#define SIG_FAKE_EVT_MIN 20000
#define SIG_FAKE_EVT_MAX 20200

/* T1REad Signals */
#define SIG_SHOW_NBEVT 5000

/* CalMonSvr signals */
#define SIG_MONIT_SEND 2000

/* Calmonsvr and Trigger2 signals */
#define SIG_START_SAVE_TO_SSD 2004 /**< @brief Save T2/monit/etc to SSD */
#define SIG_START_SAVE_T1 2003	/**< @brief Save all T1s to file */
#define SIG_START_SAVE 2002 /* Save monit/T2/muons to file */
#define SIG_STOP_SAVE  2001 /* Stop saving ... */

/* PpsIrq signals */
#define SIG_PPS_SEM_ENABLE 1001
#define SIG_PPS_SEM_DISABLE 1000

/* Mufill signals */
#define SIG_SAVE_CALIB 666
#define SIG_START_MUONRUN 777

/* Evtsvr signals */
#define SIG_EVTSVR_SEND_EVENT 7000

#endif
