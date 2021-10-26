#if !defined(_FE_DEFS_H_)
#define _FE_DEFS_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2012-03-12 11:40:54 #$
  $Revision:: 1871             $

********************************************/

#include "hardware.h"

/* Events definitions */
#define FAST_EVT_SAMPLES 1024
#define FAST_EVT_SIZE FAST_EVT_SAMPLES*8
#define MUON_EVT_SAMPLES 4096
#define MUON_EVT_SIZE MUON_EVT_SAMPLES*4

/* General Trigger Address definitions */
#define FE_SIZE 0x020000

/* Nb of FE Channels:
   Anodex30, anodex01, dynode, anodex1 ==> 4 channels
*/
#define FE_NB_CHANNELS 4
#define FE_A_30_CHANNEL 0
#define FE_A_1_CHANNEL 3
#define FE_A_01_CHANNEL 1
#define FE_D_CHANNEL 2

/* Trigger Types */
typedef enum {
  EXT_TRIGGER, SOFT_TRIGGER, TOT_A_TRIGGER, TOT_B_TRIGGER, TOT_D_TRIGGER,
  MUON_TRIGGER, IS_NOT_TRIGGER
} TRIGGER_TYPE ;

/* T2 Tags */
#define MASK_T2_TAG 0xF00000
#define T2_TAG_SHIFT 20
#define MASK_T2_MICROS 0xFFFFF
#define TOT_T2_TAG  0x800000	/**< @brief ToT T2 */
#define TOTD_T2_TAG 0x400000	/**< @brief TotD T2 */
#define GRB_T2_TAG  0x700000  /**< @brief Specific tag for GRB in the
			       T2 message */
#define STD_T2_TAG  0x100000	/**< @brief Standard (non TOT) T2 */

/* D.Nitz trigger definitions */
/* Registers Mapping */

#define FE_ID_REG 0x0 /* Identification register. yymmddxx */
#define FE_CTRL_REG 0x1 /* Global Control Register */

#define FE_SHOWER_MEM_BASE 0x2
#define FE_TRIGGER_ID_MASK 0x3
#define FE_SHOWER_MEM_STATUS 0x4

#define FE_ALL_EMPTY_BIT 0x1
#define FE_NB_BUFFER_MASK 0x70
#define FE_NB_BUFFER_SHIFT 4
#define FE_BUFFER_ADDRES_MASK 0xFFF00
#define fE_BUFFER_ADDRESS_SHIFT 8

#define FE_SOFT_REG 0x8

#define FE_TOT_A_REG 0x9
#define FE_TOT_B_REG 0xA
#define FE_TOT_THRESH_MASK 0x3FF /* 10 bits */
#define FE_TOT_THRESH_SHIFT 16
#define FE_TOT_WIDTH_MASK 0xFF /* 8 bits */
#define FE_TOT_WIDTH_SHIFT 8
#define FE_TOT_OCCUPANCY_MASK 0xFF /* 8 bits */

#define FE_TOT_D_REG 0xB /* TOT Deconvoluted - NOT IMPLEMENTED */
#define FE_SCALER_A_REG 0xE /* Scaler - NOT IMPLEMENTED */
#define FE_SCALER_B_REG 0x10 /* Sacler - NOT IMPLEMENTED */

#define FE_MUON_MEM 0x12   /* Muon buffer */
#define FE_MUON_THRESH 0x13 /* Muon threshold & enable */
#define FE_MUON_THRESH_MASK 0x3FF /* 10 bits for threshold */
#define FE_MUON_STATUS 0x14 /* Muon memory status */
#define FE_MUON_ENABLE_BIT 0x400 /* Enable is bit D10 */

/* Addresse 0x15 to 0x1F are reserved */

/* Enable Trigger bits */
#define EN_EXT_TRIGGER 0x0001
#define EN_SOFT_TRIGGER 0x0002
#define EN_TOT_A_TRIGGER 0x0004
#define EN_TOT_B_TRIGGER 0x0008
#define EN_TOT_D_TRIGGER 0x0010
#define EN_EXT_PRESCALE 0x0100
#define EN_SOFT_PRESCALE 0x0200
#define EN_TOT_A_PRESCALE 0x0400
#define EN_TOT_B_PRESCALE 0x0800
#define EN_TOT_D_PRESCALE 0x1000

#define EN_ANY_TRIGGER_MASK 0x1F1F

/* Status Trigger bits */
#define EXT_TRIGGER      0x00010000
#define SOFT_TRIGGER     0x00020000
#define TOT_A_TRIGGER    0x00040000
#define TOT_B_TRIGGER    0x00080000
#define TOT_D_TRIGGER    0x00100000
#define EXT_ADDITIONAL_TRIGGER     0x01000000
#define SOFT_ADDITIONAL_TRIGGER    0x02000000
#define TOT_A_ADDITIONAL_TRIGGER   0x04000000
#define TOT_B_ADDITIONAL_TRIGGER   0x08000000
#define TOT_D_ADDITIONAL_TRIGGER   0x10000000

#define ANY_TRIGGER_MASK (EXT_TRIGGER|SOFT_TRIGGER|TOT_A_TRIGGER|TOT_B_TRIGGER|TOT_D_TRIGGER|EXT_ADDITIONAL_TRIGGER|SOFT_ADDITIONAL_TRIGGER|TOT_A_ADDITIONAL_TRIGGER|TOT_B_ADDITIONAL_TRIGGER|TOT_D_ADDITIONAL_TRIGGER)
#define ANY_TRIGGER_SHIFT 16

#define IS_EXT_TRIGGER  1
#define IS_SOFT_TRIGGER 2
#define IS_TOTA_TRIGGER 4
#define IS_TOTB_TRIGGER 8
#define IS_TOTD_TRIGGER 0x10
#define IS_EXT_ADDITIONAL_TRIGGER 0x100
#define IS_SOFT_ADDITIONAL_TRIGGER 0x200
#define IS_TOTA_ADDITIONAL_TRIGGER 0x400
#define IS_TOTB_ADDITIONAL_TRIGGER 0x800
#define IS_TOTD_ADDITIONAL_TRIGGER 0x1000

#define IS_UNKNOWN_TRIGGER 0xFF

/* Memory Buffer Status bits */
/*19:8  = Base address of buffer
7:    = 0
6:4   = # of buffers used
3:2   = 0
1:    = All buffers full
0:    = All buffers empty
*/

#define MSTAT_BASE_BUFFER_MASK 0xFFF00
#define MSTAT_BASE_BUFFER_SHIFT 8
#define MSTAT_BUFFERS_USED_MASK 0x70
#define MSTAT_BUFFERS_USED_SHIFT 4
#define MSTAT_FULL_MASK 0x2
#define MSTAT_FULL_SHIFT 1
#define MSTAT_EMPTY_MASK 0x1

/* Nitz PLD 
     Anodex30 in low order bits of first word
     Dynode in high order bits ...
     Anodex0.1 in middle bits ...
     Anode in the low order bits of the 2nd word of
     the samples, the other bits used for debugging
*/
#define FE_ANODE_30(p) (*p & 0x3FF)
#define FE_ANODE_01(p) ((*p >> 10) & 0x3FF)
#define FE_DYNODE(p) ((*p >> 20) & 0x3FF)
#define FE_ANODE_1(p) (*(p+1) & 0x3FF)

/* Data words description */
/* V060710
Word0: (GND,VCC,GND,GND,WRT_BUFNUM[2],WRT_ADDR[10],FLAGS[4],GND,GND,ADCRD[10]
Word1: GND,GND,ADCRC[10],ADCRB[10],ADCRA[10])

Where FLAGS[3..0] are the 4 low order trigger flags at that moment.  For an external trigger, FLAGS should be 0 until the trigger point, and 1 thereafter.
*/
/* V080710
Word0: (GND,VCC,GND,GND,WRT_BUFNUM[2],WRT_ADDR[10],FLAGS[4],GND,GND,ADCRD[10]
Word1: GND,GND,ADCRC[10],ADCRB[10],DBG[10])

Where FLAGS[3..0] are the 4 low order trigger flags at that moment.  For an external trigger, FLAGS should be 0 until the trigger point, and 1 thereafter.
And DBG[10]:
$display("%x %x Word: %4d FLAGS: %x WRTADDR: %x WRTBUF: %x TRIGM[0]: %x FLAG_SET: %x NRST_FLAGS: %x INHIBIT: %x TRIGS: %x",
	    DATA2, DATA, WORDNUM, DATA2[15:12], DATA2[25:16], DATA2[27:26],
	    DATA2[11], DATA2[10], DATA2[9], DATA2[8], DATA2[4:0]);
Pas sur de bien comprendre !!!
 
*/
#define BUF_NUM_MASK      0x0C000000
#define BUF_NUM_SHIFT 26
#define BUF_ADDR_MASK     0x03FF0000
#define BUF_ADDR_SHIFT 16
#define TRIGGER_FLAG_MASK 0x0000F000
#define TRIGGER_FLAG_SHIFT 12

/*
  Specific for Muons
*/
// First word contains bit D31=1 and the time of the Muon
#define MUON_TIME_TAG_MASK 0xC0000000
#define MUON_TIME_TAG 0x80000000
#define MUON_TIME_MASK 0x3FFFFFFF

/*
  Courty FE definitions
  ATTENTION: All adresses are in Integer !
*/
#define FE_BCR_SEGMENT   0x7F  /**< @brief segment address register */
#define FE_BCR_EVENT_COUNT 0x1 /**< @brief Trigger counter */
#define FE_BCR_TEST_REG    0x3	/**< @brief Test Register */
#define FE_BCR_RESET     0x7F /**< @brief Write at this address resets the FE */
#define FE_BCR_ID_REG    0x7 /**< @brief Read at this address gets the FE ID */

#define FE_BCR_DATA      0x4000 /**< @brief segmented access to readout DATA */
#define FE_BCR_DATA_INT  0x4000
#define FE_BCR_SOFTTRIG  0x4000	/**< @brief Soft Fast trigger (EVTCLKF) */
#define FE_BCR_SOFT_SLOW 0x4001	/**< @brief Soft Slow Trigger (EVTCLKS) */

#define FE_BCR_IDENT 0x46454546	/**< @brief Courty Ident */

#define FE_BCR_ANODE_30(p) ((*p >> 18) & 0x3FF)
#define FE_BCR_ANODE_01(p) ((*p >> 8) & 0x3FF)
#define FE_BCR_DYNODE(p) (((*p & 0xFF) << 2) | ((*(p+1) >> 30) & 0x3))
#define FE_BCR_ANODE_1(p) ((*(p+1) >> 20) & 0x3FF)

enum {
  FE_UNKNOWN_TYPE, FE_NITZ_TYPE, FE_COURTY_TYPE
} ;


#define fereg(x) (unsigned int *)( FeBase + (x) )

/*
  Test presence of the FE Board, read 32 bits at BASE_SC + 1FC
  Actually, as the base is 32 bits (unsigned int), the offset is 0x7F
*/
#define FE_PRESENCE_OFFSET 0x7F
#define FE_PRESENCE_MASK 0x00FF0000
#define FE_IS_ABSENT 0x00F00000
#define FE_IS_PRESENT  0x000F0000

/*
  Tasks involved in T1 Events: [T1Fake], T1Irq, T1Read, Trigger2:
  T1Fake : generates a Soft FAST IRQ [/EVTCLKF]. In addition increments
           RT semaphore "T1Fake" (for test purpose, never decremented)
	   T1Fake is a Xenomai Task.
	   T1Fake is only used if there is NO Front End Board.
  T1Irq  : t1irq task makes a 'V' on "FastIrqSem" semaphore upon reception
           of an EVTCLKF Interrupt.
           FastIrqSem is a RT Xenomai semaphore.
	   T1Irq is a Xenomai task.
  T1Read : t1ready makes a 'P' on FastIrqSem and makes a 'V' on "FastReadySem"
           after having read the event and the Time stamp.
	   FastReadySem is a Linux semaphore.
	   T1Read is a Xenomai task.
  Trigger2: Makes a V on FastReadySem.
            Trigger2 is a pure linux task (no RT there)
*/
#define FAST_READY_TASK_NAME "FastReadyTask"
#define FAST_READY_SEMAPHORE "FastReadySem"

#define T1_FAKE_SEMAPHORE "T1Fake"

/*
  Same thing for Muons: [MuFake],MuIrq, MuRead, Mufill:
  MuFake : generates a Soft SLOW IRQ [EVTCLKS]. In addition increments
           RT semaphore "MuFake" (for test purpose, never decremented)
	   MuFake is a Xenomai Task.
  MuIrq  : muirq task makes a 'V' on "SlowIrqSem" upon reception of an 
           EVTCLKS Interrupt.
           SlowIrqSem is a RT Xenomai semaphore.
	   MuIrq is a Xenomai task.
  MuRead:  muread makes a 'P' on SlowIrqSem and makes a 'V' on "SlowReadySem"
           semaphore after having read the event and the Time stamp.
	   SlowReadySem is a Linux sempahore.
	   MuRead is a Xenomai task.
  MuFill:  mufill makes a 'P' on SlowReadySem.
           MuFill is a pure Linux task (no RT there)


*/

#define SLOW_READY_SEMAPHORE "SlowReadySem"
#define SLOW_READY_TASK_NAME "SlowReadyTask"

#define MU_FAKE_SEMAPHORE "MuFake"

#endif
