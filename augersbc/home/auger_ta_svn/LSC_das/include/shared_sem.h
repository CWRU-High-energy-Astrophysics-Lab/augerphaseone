#if !defined(_SHARED_SEM_H_)
#define _SHARED_SEM_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-01-19 14:42:46 #$
  $Revision:: 529              $

********************************************/


typedef struct {
  char *name ;
  unsigned int key ;
} shared_sem_t ;

shared_sem_t SharedSem[] = {
  {"EvtBuffer", 0x27121042},
  {"FastBuffer", 0x12156661},
  {"??", 0x063c2e40},
  {"monitbuf", 0x0f1b0d19},
  {"???", 0x652b5535},
  {NULL, 0}
} ;

/*
0x27121042 0          root      666        1         
0x12156661 32769      root      666        1         
0x063c2e40 65538      root      666        1         
0x0f1b0d19 98307      root      666        1         
0x652b5535 131076     root      666        1         


*/

#endif
