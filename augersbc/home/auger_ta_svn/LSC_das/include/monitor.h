#if !defined(_MONITOR_H_)
#define _MONITOR_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-09-27 10:51:48 #$
  $Revision:: 1567             $

********************************************/

/* Monitor Block */
typedef struct {
  unsigned int time ;		/**< @brief Time in second */
  int gps_utc_offset ;		/**< @brief Offset between GPS and UTC time */
  int count ;			/**< @brief Nb of items used in averaging */
  int adc[MAX_NB_ADC] ;         /**< @brief Averages of ADC values */
  int adc_min[MAX_NB_ADC] ;
  int adc_max[MAX_NB_ADC] ;
  unsigned int dac ;		/**< @brief Dac value - No way to read back ! */
  int pcb_th,		/**< @brief Temperature of the PCB (SHT11) */
    pcb_rh ;		/**< @brief Humidity ... */
  int pcb_th_min,
    pcb_th_max,
    pcb_rh_min,
    pcb_rh_max ;
  // TPCB Data
  int tpcb_panel_current,	/**< @brief panel current in mAmps */
    tpcb_panel_voltage,		/**< @brief panel voltage in mVolts */
    tpcb_load_current,		/**< @brief load current in mAmps */
    tpcb_load_voltage,		/**< @brief load voltage in mVolts */
    tpcb_pressure,		/**< @brief pressure 10th hpa */
    tpcb_temperature ;		/**< @brief temperature 10th degrees */

  // Radio monitoring data - Preliminary
  int lr_data[MAX_LR_MONIT_DATA] ;

} MONITOR_BLOCK ;

typedef struct {
  time_t last_update ;		/**< @brief Date of last update */
  int count ;			/**< @brief Nb of monitoring measures averadged */
} mon_any_t ;

/* Definitions for the monitor circular buffer (library BMLib) */
#define MONITOR_NB_OF_BLOCKS 10
#define MONITOR_BUFFER_NAME "monitbuf"
#define SCALE_FACTOR 1000.

#endif
