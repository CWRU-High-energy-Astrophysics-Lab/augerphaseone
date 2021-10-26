#if !defined(_FE_MAPPING_H)

#define _FE_MAPPING_H


typedef struct {
  char * name ;
  unsigned int start, size ;
} REGISTER_INFO ;

static REGISTER_INFO RegInfo[] = {
  {"FE ID", FE_ID_REG, 1},
  {"Global Ctrl", FE_CTRL_REG, 1},
  {"Shower Mem", FE_SHOWER_MEM_BASE, 3},
  {"Soft Trig", FE_SOFT_REG, 1},
  {"ToT A", FE_TOT_A_REG, 1},
  {"ToT B", FE_TOT_B_REG, 1},
  {"ToT D", FE_TOT_D_REG, 2},
  {"Scaler A", FE_SCALER_A_REG, 2},
  {"Scaler B", FE_SCALER_B_REG, 2},
  {"Muon Block", FE_MUON_MEM, 5},
  {NULL, 0, 0}
} ;

#endif
