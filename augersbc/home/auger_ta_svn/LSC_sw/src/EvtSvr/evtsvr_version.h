/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-08-25 14:44:03 #$
  $Revision:: 1439             $

********************************************/

/*

  Creation - guglielm - 2009/12/16

  History
  V1 - LGG - 2009/12/16
    Creation
  V2 - LGG - 2011/01/11
    Fixed bug in search event. Now event is found
  V3 - LGG - 2011/01/11
    Send the Event Data (partial, including Monitoring and GPS sawtooth.
    Calibx data still missing.
    Empty calibh data
  V4 - LGG - 2011/01/22
    Use calibx
  V5 - LGG - 2011/08/25
    Added use of signal ??? to send the newest event
  V6 - LGG - 2013/04/24
    Added calibx data
  V7 - LGG - 2016-09-14
    Fixed bug in size of T3 event buffer (missing Calib data)
*/

#if !defined(_EVTSVR_VERSION_H)

#define _EVTSVR_VERSION_H

#define EVTSVR_VERSION 7

#endif
