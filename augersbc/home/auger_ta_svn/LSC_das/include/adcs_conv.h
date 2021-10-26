#if !defined(_ADCS_CONV_H_)
#define _ADCS_CONV_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2010-11-19 15:57:27 #$
  $Revision:: 249              $

********************************************/

/*
  Min and Max are used to check that the ADC is working correctly and
  that some Voltages/Currents are within min and max (normal values).
  If min == -1, ttadc does not check.
   A special case is the FrontEnd measured current:
     if the FE is absent the current should be 0.
     if the FE is present, the current should be >= max.
    --> the FE current is not usable to check the ADC
   If the measured current is between min and max, that's OK, otherwise
   there is something wrong, either on the ADC itself or somewhere else.
*/

typedef struct {
  char *name ;			/**< Channel Name */
  int idx ;			/**< Index in the array of ADC channels read */
  float convert ;		/**< Transfert function ( y = x*convert ) */
  float min ;			/**< Minimal value expected (ttadc) */
  float max ;			/**< Maximal value expected (ttadc) */
} ONE_ADC_STRUCT ;

/* FE 12V current with Front End */
#define I_FE_YES .1
#define I_FE_NO   0.

#if defined(ADC_MAX1231)
/*******************************************************
  For Proto #1, with MAX 1731 ADC
*******************************************************/

static ONE_ADC_STRUCT AdcsConv[MAX_NB_ADC] = {
  {"TEMP", TEMP_IDX, 0.125, 20., 50.},	/**< 1/8 of degre per bit */
  {"I-BATT", IBATT_IDX, 1./2.5, -1., -1.},	/**< Battery current */
  {"I-VCC", ICC_IDX, 1./2.5, -1., -1. },	/**< VCC current */
  {"I-FE-12V", IFE12V_IDX, 1., -1., I_FE_YES},	/**< FE 12V Current */
  {"V-VDD-1.8V", VDD_IDX, 5., -1., -1.},
  {"V-VCC-3.3V", VCC_IDX, 5., -1., -1.}, /**< VCC Voltage - 3.3 V */
  {"V-Fpga-1.2V", VFPGA_IDX, 5., -1., -1.}, /**< FPGA Voltage - 1.2 V */
  {"V-FE-12V", VFE12V_IDX, 11, 10., 14.},
  {"V-BATT", VBATT_IDX, 11., 10., 14.},	/**< Battery Voltage - around 12V */
  {"FE-1", FE_1_IDX, 1., -1., -1.},		/**< From FE:  */
  {"FE-2", FE_2_IDX, 1., -1., -1.},		/**< From FE:  */
  {"FE-3", FE_3_IDX, 1., -1., -1.},		/**< From FE:  */
  {"FE-4", FE_4_IDX, 1., -1., -1.},		/**< From FE:  */
  {"P5V", P5V_IDX, 2., -1., -1.}
} ;

#elif defined(ADC_AD7490)
/*******************************************************
  For Proto #2, with AD7490 ADC
*******************************************************/

static ONE_ADC_STRUCT AdcsConv[MAX_NB_ADC] = {
  {"FE-1", FE_1_IDX, 2., -1., -1.},		/**< From FE:  */
  {"FE-2", FE_2_IDX, 2., -1., -1.},		/**< From FE:  */
  {"FE-3", FE_3_IDX, 2., -1., -1.},		/**< From FE:  */
  {"FE-4", FE_4_IDX, 2., -1., -1.},		/**< From FE:  */
  {"V-BATT", VBATT_IDX, 11., 10., 14.},	/**< Battery Voltage - around 12V */
  {"V-FE-12V", VFE12V_IDX, 11., 10., 14.},
  {"V-Fpga-1.2V", VFPGA_IDX, 1., 1.1, 1.4}, /**< FPGA Voltage - 1.2 V */
  {"V-VCC-3.3V", VCC_IDX, 2., 3.1, 3.5}, /**< VCC Voltage - 3.3 V */
  {"V-CORE-1.8V", VCORE_IDX, 1., 1.7, 1.9},
  {"I-FE-12V", IFE12V_IDX, 1./2.5, 0., .2},	/**< FE 12V Current */
  {"I-VCC", ICC_IDX, 1./2.5, -1., -1. },	/**< VCC current */
  {"I-BATT", IBATT_IDX, 1./2.5, -1., -1.},	/**< Battery current */
  {"V-5V", V5V_IDX, 2.2, -1., -1.}
} ;

#else
#error No valid ADC defined
#endif

#endif
