#if !defined(_CALIBH_H_)
#define _CALIBH_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-01-21 17:15:25 #$
  $Revision:: 562              $

********************************************/


#define NB_HISTO_CALIB 10
#define SINGLE_MUON_SIZE 20

typedef struct {
/*** CALIB informations to be sent */
  unsigned short hbase[NB_HISTO_CALIB];
  unsigned short base[20];
  unsigned short peak[150];
  unsigned short charge[4][600];
  unsigned int shape[SINGLE_MUON_SIZE];
} CALIBH_BLOCK;

#if 0
// Not used int AS
typedef struct {
  unsigned int requestId;
  unsigned int bufferOk;
  CALIBH_BLOCK calibh ;
} calibh_block_t ;
#endif

#define MAX_CALIBH_BUFFER  20
#define CALIBH_BUFFER_NAME "CalHBuff"

#endif
