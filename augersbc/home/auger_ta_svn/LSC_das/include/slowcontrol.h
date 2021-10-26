#if !defined(_SLOWCONTROL_H_)
#define _SLOWCONTROL_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-06-08 14:51:37 #$
  $Revision:: 1178             $

********************************************/
/**
 * @defgroup slowcontrol_h Slow Control Definitions 
 * @ingroup services_include
 *
 */

/**@{*/

/**
 * @file   slowcontrol.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   8/07/2009
 * 
 * @brief  Slow Control Definitions
 * 
 * 
 */

/*
  Proto #2
*/
#define ADC_AD7490

/*
  Proto #1
*/
//#define ADC_MAX1231

#if defined(ADC_MAX1231)
#define MAX_NB_ADC 14
#else
#define MAX_NB_ADC 13
#endif

/*
  Proto #2
*/
#define DAC_DAC7554

/*
  Proto #1
*/
//#define DAC_MAX5306

int SlowlibVersion() ;
int SlowlibVerbose( int verbose ) ;

int AdcConvertUsec( int usec ) ;
int AdcSetReset( int flag ) ;
int AdcRead(float * padc ) ;
int AdcOpen() ;
void AdcClose() ;
int DacOpen() ;
void DacClose() ;
int DacSet( int channel, unsigned short value ) ;
int DacSetVolts( int chan, float volts ) ;

int ThRhOpen() ;
void ThRhClose() ;
int ThRhRead( float * temp, float * humid ) ;
void ThRhSetWait( int micros ) ;

int HvToDac( float hv ) ;
float DacToHv( unsigned int dac ) ;

/**@}*/

#endif
