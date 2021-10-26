#if !defined(_TTADC_H)
#define _TTADC_H


/***********************************
 ************* ADC Part   **********
 ************************************/
/**
 * @defgroup adcs_defs_h Slow Control ADC and DAC definitions 
 * @ingroup services_include
 *
 */

/**@{*/

/**
 * @file   adcs_defs.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   8/07/2009
 * 
 * @brief  Definitions for Slow Control ADC and DAC
 * 
 * 
 */

#include "slowcontrol.h"

#define V_CONV (2.5/4095.)
#define ADU_TO_VOLTS V_CONV

#if defined(ADC_AD7490)
/*******************************************************
  For Proto #2, with AD7490 ADC
*******************************************************/
/*
Les canaux A et B du DAC (DAC7554) sont les consignes envoyees au FE pin 5 et 6
Je ne sais plus laquelle des deux ils utilisent puisqu'il n'y a qu'un seul PM et une seule HT.
Vref= 2.5V

Pour l'ADC (AD7490)
Vref=2.5V

Ch0: fe1, monitoring FE pin21
Ch1 : fe2, monitoring FE pin22
Ch2 : fe3, monitoring FE pin25
Ch3 : fe4, monitoring FE pin26
Ch4 : Vbatt , Vbatt= 11*Vch4
Ch5 : Vfe (=Vbatt), Vfe = 11*Vch5
Ch6 : Vfpga (1.2V)
Ch7 : Vcc (3.3V), Vcc = 2*Vch7
Ch8 : Vcore (1.8V)
Ch9 : Ife , courant consomme par le FE, Ife = Vch9/2.5
Ch10 : Icc , courant sur le 3.3V, Icc = Vch10/2.5
Ch11 : Ibatt , courant consomme par la DAQ (Hors FE), Vch11 = 50 * 0.05 * Ibatt, soit Ibatt = Vch11/2.5
Ch12 : V5v (5V), V5v = 2.2 *Vch12
Ch13-15 not used
 */
/* The index is the index in the channels read (17 = Temp + 16 external) */
enum {
  FE_1_IDX,			/**< From FE:  */
  FE_2_IDX,			/**< From FE:  */
  FE_3_IDX,			/**< From FE:  */
  FE_4_IDX,			/**< From FE:  */
  VBATT_IDX,			/**< Tension Batt 12V */
  VFE12V_IDX,			/**< Tension FE 12V */
  VFPGA_IDX,			/**< FPGA Core 1.2 V */
  VCC_IDX,			/**< Vcc 3.3 V */
  VCORE_IDX,			/**< Tension CPU Core (1.8) */
  IFE12V_IDX,			/**< Courant FE 12V */
  ICC_IDX,			/**< Courant Vcc */
  IBATT_IDX,			/**< Courant Battery */
  V5V_IDX,			/**< 5 Volts, divided by ? */
  U1,
  U2,
  U3
} ;

#define WRITE_BIT 0x800 /* D11 */
#define SEQ_BIT 0x400
#define ADD_00 0x000   /* Channel 0 */
#define PM_FULL 0x030
#define SHADOW_BIT 0x008
#define WEAK_BIT 0x004
#define RANGE_EXTENDED 0x002
#define CODING_BINARY 0x001

#define ADC_INITIALIZE 0xFFFF

#define ADC_CONTROL_0 WRITE_BIT | SHADOW_BIT | PM_FULL | \
   WEAK_BIT | RANGE_EXTENDED | CODING_BINARY

#define ADC_CONTROL_1 WRITE_BIT | SEQ_BIT | PM_FULL | \
   WEAK_BIT | RANGE_EXTENDED | CODING_BINARY

#define ADC_STOP_CONVERSION WRITE_BIT | PM_FULL | WEAK_BIT | \
   RANGE_EXTENDED | CODING_BINARY

#define ADC_VALUE_MASK 0xFFF
#define ADC_CHAN_SHIFT 12

/* 13 channels, 0 to 12 */
#define ADC_SHADOW 0xFFF8
#else
#error No valid ADC defined
#endif

/***********************************
 ************* DAC Part   **********
 ************************************/

#if defined(DAC_DAC7554)
/*******************************************************
  For Proto #2, with DAC7554 DAC (from TI)
*******************************************************/

#define DAC_RANGE 4095
#define DAC_MAX_VOLTAGE 2.5
/*
  Conversion DacV to HV (measured):
    0.999 --> 825
    2. --> 1454
    1.65 --> 1200
*/

#define HV_TO_DAC_VOLTS (1./825.)
#define VOLT_TO_DDU (4095/DAC_MAX_VOLTAGE)
#define DDU_TO_VOLT ((float)DAC_MAX_VOLTAGE/4095.)

/* Input register, Dac register, output Updated */
#define DAC_REG_0 0x8000
#define DAC_REG_1 0x9000
#define DAC_REG_2 0xA000
#define DAC_REG_3 0xB000

/* All dac in Hi-Z */
#define DAC_REG_PWD 0xFFFF

#define DAC_HV_CHANNEL 0 /* a verifier */

#else
#error Not a Valid DAC
#endif

/**@}*/

#endif
