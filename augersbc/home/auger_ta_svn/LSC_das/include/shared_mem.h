#if !defined(_SHARED_MEM_H)

#define _SHARED_MEM_H

enum {
  IS_ACQ_SHM, IS_SRV_SHM
} ;

typedef struct {
  char *name ;
  unsigned int key ;
  int acq_srv ;
} shared_mem_t ;

shared_mem_t SharedMem[] = {
  /* Specific Acquisition*/
  {"AcqConfig", 0x2a170d49, IS_ACQ_SHM},
  {"acqstatus", 0x06050266, IS_ACQ_SHM},
  {"FastBuffer", 0x12156661, IS_ACQ_SHM},
  {"EvtBuffer", 0x27121042, IS_ACQ_SHM},
  {"monitbuf", 0x0f1b0d19, IS_ACQ_SHM},
  /*Services and Acquisition*/
  {"GpsStatus", 0x26071140, IS_SRV_SHM},
  {"svrstatus", 0x06061774, IS_SRV_SHM},
  {"TpcbStatus", 0x16027772, IS_SRV_SHM},
  {NULL, 0, IS_SRV_SHM}
} ;

#endif

