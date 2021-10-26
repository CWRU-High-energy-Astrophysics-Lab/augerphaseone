#if !defined(_TPCB_H)

#define _TPCB_H

#define TPCB_STATUS_NAME "TpcbStatus"

typedef struct {
  unsigned int initialized ;
  time_t last_update ;
  int panel_current ;		/**< mAmps */
  int panel_voltage ;		/**< mVolts */
  int load_current ;
  int load_voltage ;
  int pressure ;		/**< .1 Hpa */
  int temperature ;		/**< .1 Centigrades */
} TPCB_STATUS ;

#define PANEL_CURRENT_PER_BIT 5.  /* mAmp */
#define PANEL_VOLTAGE_PER_BIT 30. /* mV */ 
#define PANEL_CURRENT_NAME "TPCB-P-I"
#define PANEL_VOLTAGE_NAME "TPCB-P-V"

#define LOAD_CURRENT_PER_BIT 1.  /* mAmp */
#define LOAD_VOLTAGE_PER_BIT 19. /* mV */
#define LOAD_CURRENT_NAME "TPCB-L-I"
#define LOAD_VOLTAGE_NAME "TPCB-L-V"

#define PRESSURE_MILLIBAR_PER_BIT .1
#define TEMP_DEGRE_PER_BIT .1 /* Celsius ! */
#define TPCB_PRESSURE_NAME "TPCB-AP"
#define TPCB_TEMPERATURE_NAME "TPCB-T"

#define TPCB_POWER_CAN_ID 0x100
#define TPCB_TEMPERATURE_CAN_ID 0x101
#define TPCB_CAN_MASK 0x100

#endif
