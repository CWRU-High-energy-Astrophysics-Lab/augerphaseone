#if !defined(_ACQ_VERSION_H_)
#define _SVR_VERSION_H_

/*******************************************

 $Author:: guglielmi          $
 $Date:: 2011-03-04 12:12:19 #$
 $Revision:: 732              $

********************************************/


/**
 * @defgroup services_include Services Include Files
 *
 */

/**
 * @defgroup svr_version_h Version of Services
 * @ingroup services_include
 *
 * The version number is set in the acqstatus shared memory.
 * The format is:
 *  - $YY: Year
 *  - $MM: Month
 *  - $DD: Day
 *  - $xx: 
 *        - 00 -> Partial version
 *        - 01 -> Almost complete (includes Trigger Firmware)
 *        - 10 -> Complete version (includes Trigger Firmware and calibration)
 *        .
 * Example: 0x11020100: Feb 01 2011, partial version
 * .
 * 
 *  *
 * History
 *   - 0x11021100: Everything in place, running. Not Fully tested !
 *   - 0x11041800: Fully running. Radio protocole not completed.
 *   - 0x11041900: Preliminary implementation of radio protocole. To
 *     be checked with D.Nitz and Roger.
 *   - 0x11061000: Ready for installation test at Golden/Lamar
 *   - 0x11070600: Some bug fixes.
 *   - 0x11070700: Hopefully fixed some bad errors with "srv restart"
 *   - 0x11071900: Preparing for D.Nitz trigger firmware
 *                 Fixing some issues in 'srv stop/restart'
 *   - 0x11080100: Various changes/bug fixes/etc
 *   - 0x11080200: Added Acknowledge handling in msgsvr (preliminary)
 *   - 0x11080400: Same as 11080200, just for test
 *   - 0x11081500: Just a test
 *   - 0x11081600: Fix an alignment problem in M_READY message
 *                  structure: utc_offset now a short, just before the
 *                  software version.
 *   - 0x11082500: Added signal SIG_EVTSVR_SEND_EVENT to send the
 *                 newest available event (evtsvr)
 *   - 0x11082600: Preliminary handling wireless status
 *   - 0x11083000: Wireless status handling implemented (and partly
 *                  tested)
 *   - 0x11090200: Modified Radio Monitoring data structure.
 *   - 0x11090900: Added usage of Run Conditions file (runcond.cfg)
 *   - 0x11091200: Added signals (for Golin)
 *   - 0x11092200: Added Radio Monitoring Data Handling
 *   - 0x11100300: Fixed bug in lstest
 *   - 0x11101400: Suppressed prints in Canlib
 *   - 0x11102000: Added function MsgSvrClientGeneric (send ascii msg
 *                 to CDAS)
 *                 Cleanup control and gpsctrl accordingly
 *   - 0x11102400: Some cleanup. Golin + getrate seem to work. Need
 *                 D.Nitz trigger to really test
 *   - 0x11102600: Fixed a problem in function 'is_running' when
 *                 called with the process name (instead of the full name like
 *                 "/root/LSC/bin/process"
 *   - 0x11102700: Fixed minor problems in libutil (is_running and
 *                  get_exit_code)
 *   - 0x11110400: Cleanup and allow abbreviations in buildconfig
 *   - 0x11110700: Added 'setcpunb'
 *   - 0x11111400: Seveeal cleanups, bug fixes.
 *   - 0x11111600: No change, just to keep the same version nb as Acq
 *   - 0x11112200: Few changes, mostly cleanups.
 *   - 0x11112500: No real change, but Trigger firmware now includes
 *                 External Trigger.
 *   - 0x11112801: Bug fix in acq (gocalib + golin)
 *   - 0x11121201: Cleanup logkeep and chamges in Acq
 *   - 0x12011800: Many changes, bug fixes.
 *   - 0x12031001: Fixes in das (golin, etc)
 *   - 0x12071001: Added almanach handling: load an almanach at pwron,
 *                 save the almanach when a new one is detected.
 *   - 0x12083100: Almanach file now on usb file system (not ram0)
 *   - 0x12110301: Cleanups.
 *   - 0x13032100: Cleanups.
 *   - 0x13032800: Cleanups.
 *   - 0x13040801: Cleanups. Bugs fixes in ACQ.
 *   - 0x13041501: Signals added to sigauger.
 *   - 0x13041701: Nothing new.
 *   - 0x13041910: Fixed bug in GetCpunumber function.
 *   - 0x14022201: Cleanups. Added signal 2004 to save events to big key/ssd
 */
/**@{*/

/**
 * @file   svr_version.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Fri Feb 11 14:37:21 CET 2011
 * 
 * @brief   Version of the Services
*/

#define SVR_VERSION 0x14022201

/**@}*/

#endif
