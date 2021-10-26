#if !defined(_LEDLIB_H_)
#define _LEDLIB_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-02 16:46:00 #$
  $Revision:: 1691             $

********************************************/


/**
 * @defgroup ledlib_h Led Flasher Library
 * @ingroup acq_include
 */
/**@{*/

/**
 * @file   ledlib.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Tue Mar  8 14:56:42 CET 2011
 * 
 * @brief  Led Flasher Library
*/

/*
  Vext: 0-2.5 V
  Base: 0-10 V
  Dac: 0-10 V
*/

enum {
  LEDLIB_OK, LEDLIB_CANT_OPEN_CAN, LEDLIB_CANT_SET_FILTER,
  LEDLIB_LEF_NOT_READY, LEDLIB_UNKNOWN_CHANNEL, LEDLIB_TOO_MANY_TRIG,
  LEDLIB_OUT_OF_RANGE, LEDLIB_BAD_ACK, LEDLIB_LAST_ERROR
} ;

typedef enum {
  VEXT_CHANNEL, A_BASE_CHANNEL, A_DAC_CHANNEL, B_BASE_CHANNEL, B_DAC_CHANNEL
} LEF_CHANNELS ;

enum {
  LED_A_BASE, LED_B_BASE, LED_A_DAC, LED_B_DAC
} ;

#define LEF_TRIG_HIGH 0x1
#define LEF_TRIG_FIRE 0x2
#define LEF_TRIG_LOW 0x0

/* PWM voltage max */
#define MAX_PWM_VOLTS 5.
#define VEXT_MAX_VOLTS 2.5
#define LED_BASE_MAX_VOLTS 10.
#define LED_DAC_MAX_VOLTS 10.

#define VEXT_MIN_VOLTS 0.02
#define VEXT_INC_VOLTS 0.02

#define MAX_OUTPUT_VOLTAGE 11.

#define A_DAC_MAX 0xFF
#define B_DAC_MAX 0xFF

/* Timeout while waiting for an ACK in seconds */
#define MAX_TIMEOUT 5

/* DAC Types */
enum {
  MAX520_DAC, AD5316_DAC
} ;

/* AD5316 Slave address and Range */
#define LEF_AD5316_SLAVE 1
#define AD5316_RANGE 0x3ff

/* MAX520 Slave Address and Range */
#define LEF_MAX520_SLAVE 0x50
#define MAX520_RANGE 0xff

/* Channels in the MAX520 */
enum {
  MAX520_BASE_A, MAX520_BASE_B, MAX520_DAC_A, MAX520_DAC_B
} ;

enum {
  LEVEL_LOW, LEVEL_HIGH, LEVEL_PULSE
} ;

/* Max nb of shots per LEF_TRIG_MULTI request */
#define MAX_SHOTS_FIRE 0xFF

int LedlibVersionNb() ;
char * LedlibVersion() ;
char * LedlibErrStr( int err ) ;
void LedlibSetVerbose( int level ) ;
void LedlibSetTestmode( int mode ) ;

int LedlibOpen() ;
void LedlibClose() ;
int LedlibEchoRequest( unsigned char * msg, int ll ) ;
int LedlibSetVextVolt( float volts ) ;
int LedlibSetVolt( int channel, float dac ) ;
int LedlibEnablePps( int level ) ;
int LedlibFireOne( int level ) ;
int LedlibFire( int count, int duration ) ;

int LedlibSetDac( int channel, int dac ) ;
int LedlibSetAllDac( int vext, int a_base, int a_dac, int b_base, int b_dac ) ;
int LedlibSetAllVolt( float vext, float a_base, float a_dac, float b_base, float b_dac ) ;

int LedlibVoltToDac( int channel, float volts ) ;
float LedlibDacToVolt( int channel, int adu ) ;
float LedlibTotalVoltage( float vext, float base, float dac ) ;

/**@}*/

#endif
