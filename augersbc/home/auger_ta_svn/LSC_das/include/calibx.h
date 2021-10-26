#if !defined(_CALIBX_H_)
#define _CALIBX_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2012-01-18 11:48:50 #$
  $Revision:: 1842             $

********************************************/

#define CALIB_XAV_VERSION 18  /* Internal calibration version value */

//#define NB_HISTO_CALIB 5 // The 'real' nb TBD

typedef struct {
/*** CALIB informations to be sent */
  unsigned short version;
  unsigned int start_second;
  unsigned int end_second;
  unsigned short nb_trigger;     /**< @brief used for this computation */
  unsigned short nb_t2;
  unsigned short past;
  unsigned short base[FE_NB_CHANNELS];        /**< @brief 100*base */
  unsigned short sigma_base[FE_NB_CHANNELS];  /**< @brief 10*n*sigma2 base */
  unsigned short vem;         /**< @brief 10*vem */
  int tube_on ;			/**< @brief 0 if PMT is down, 1 otherwise */
  unsigned short rate;        /**< @brief 100*rate in Hz */
  unsigned short nb_trigger_ad;
  unsigned short dynode_anode;  /**< @brief 100*(dynode/anode) */
  unsigned short sigma_dynode_anode; /**< @brief 100*sigma (dynode/anode) squared */
  unsigned short area[FE_NB_CHANNELS];        /**< @brief 10*area */
  unsigned short nb_tot;
  unsigned short da_offset; /**< @brief 100*offset between dynode/anode */
  unsigned short sigma_da_offset; /**< @brief 100* sigma_da_offset squared */
  unsigned short da_chi2; /**< @brief 100*the chi-squared for the da fit */
} CALIBX_BLOCK;

#if 0
// Not used int AS
typedef struct {
  unsigned int requestId;
  unsigned int bufferOk;
  CALIBX_BLOCK calibx ;
} calibx_block_t ;
#endif

#define MAX_CALIBX_BUFFER  20
#define CALIBX_BUFFER_NAME "CalXBuff"

#endif
