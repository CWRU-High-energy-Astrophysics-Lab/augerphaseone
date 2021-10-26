#if !defined(_ACQ_MSG_H_)

#define _ACQ_MSG_H_

/**
 * @defgroup acq_msg  Specific Acquisition Message definitions
 * @ingroup acq_include
 *
 */

/**@{*/
/**
 * @file   acq_msg.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Wed Feb  9 15:30:31 2011
 * 
 * @brief  Specific Acquisition Message definitions
 * 
 * 
 */

/**
 * @struct T2_MESSAGE
 * @brief Message containing the T2 time stamps, sent every second.
 *
 * T2_MESSAGE is made of:
 *    - 32 bits: The GPS Second of the following T2 time stamps.
 *    - For each timestamp
 *        - 4 bits: the event type.
 *            - 0x7 is specific to GRB data.
 *        - 20 bits: the time in <B>microseconds</B> relative to 
 *          the GPS second.
 *
 */
typedef struct {
  int seconds ;
  unsigned char the_t2s[2] ;
} T2_MESSAGE ;

#define EVTID_MASK 0x3FFF
#define EVTID_AGAIN_SHIFT 14
#define EVTID_AGAIN_MASK 0x3


/**
 * @struct T3_YES_MESSAGE
 * @brief Message received from CDAS
 *
 * T3 Request Message from CDAS. The message is made of
 *    - evtid: Event ID
 *    - again: If the same event is requested again
 *    - time: time stamp in seconds and microseconds
 *    - microref: offset to the timestamp
 *    - delta: search between timestamp-delta and timestamp+delta
 *
 */
typedef struct {
  int evtid ;
  int again ;
  ONE_TIME time ;
  unsigned char microref ;
  unsigned char delta ;
} T3_YES_MESSAGE ;

/**
 * @struct T3_EVT_MESSAGE_HEADER
 * @brief T3 Message Header
 * 
 * - evtid: Event ID (from CDAS T3 Request)
 * - comprssed: 1 if compressed, 0 othewise
 * - error: Error code
 */
typedef struct {
  unsigned short evtid ;
  char compressed ;
  char error ;
} T3_EVT_MESSAGE_HEADER ;

/**
 * @struct T3_EVT_MESSAGE
 * @brief T3 Message
 *
 * The data part of the message is made of
 *   - The Fast Event
 *   - CalibX data (Not Implemented Yet)
 *   - CalibH data (Not Implemented Yet)
 *   - Ttag data (see below t3_ttag.h file)
 *
 */
typedef struct {
  T3_EVT_MESSAGE_HEADER header ;
  unsigned char data[4] ;
} T3_EVT_MESSAGE ;

/**@}*/

#endif
