/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-08-26 16:52:58 #$
  $Revision:: 1451             $

********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


/**
 * @defgroup acklist_c  Handling Stream UP Acknowledge
 * @ingroup msgsvr
 *
 * Handling Stream UP Acknowledge
 */

/**@{*/
/**
 * @file   acklist.c
 * @author guglielm
 * @date   2011/08/02
 * 
 * @brief  Handling Stream UP Acknowledge
 *
 * In principle, the LR sends back an UP ACK frame within the second when
 * it receives the data stream End Frame. The msgsvr should not send any
 * other data stream untill the ACK of the previous one has been received.
 *
 * It is not clear (yet) what to do in case the ACK is received late.
 *
 *
 */
#include "can.h"

#include "msgsvr_version.h"
#include "msgsvr.h"
#include "central_local.h"
#include "acklist.h"

/**************************************************
  Static Variables
***************************************************/

/**************************************************
  Global Variables
***************************************************/
extern int Verbose ;

/**************************************************
  Static Functions
***************************************************/

/**************************************************
  Global Functions
***************************************************/

static int NbAck = 0 ;		/**< @brief Current nb of UP ACK expected */

/** 
 * Increment the number of expected UP ACK. Should never be higher than 1.
 * 
 * 
 * @return The actual number of Up Stream Acknowledge that are expected.
 */
int AddAck()
{
  NbAck++ ;
  return NbAck ;
}

/** 
 * Decrement the number of expected UP ACK. Should never be less than 0.
 * 
 * 
 * @return The actual number of Up Stream Acknowledge that are expected.
 */
int RemAck()
{
  NbAck-- ;
  return NbAck ;
}

/** 
 * Return the number of expected UP ACK. Should never be less than 0, nor
 * higher than 1 !
 * 
 * 
 * @return The actual number of Acknowledge that are expected.
 */
int AnyAck()
{
  return NbAck ;
}

/**@}*/
