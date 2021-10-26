#if !defined(_HISTO_H_)

/*******************************************************

  $Author: guglielmi $
  $Revision: 548 $
  $Date: 2011-01-20 16:40:36 +0100 (Thu, 20 Jan 2011) $

  $Log

********************************************************/

#define _HISTO_H_

#define HISTO_SYNC 0xDEDEBABA

#include <time.h>

typedef struct {
  unsigned int Sync ;
  time_t begin ;
  int First, Last, Width, Nbins ;
  int Entries, Uflow, Oflow, Min, Max ;
  char Title[80] ;
  int *Histo ;
} HISTO ;

HISTO * HistInit( char *title, int size, int first, int last ) ;
void HistReset( HISTO *ph ) ;
int HistAdd( HISTO *ph, int value ) ;
int HistPrint( HISTO *ph, char *fname ) ;
int HistSave( HISTO *ph, char *fname ) ;

#endif
