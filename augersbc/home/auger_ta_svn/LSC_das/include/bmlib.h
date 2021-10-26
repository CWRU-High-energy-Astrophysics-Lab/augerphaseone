#if !defined(_BMLIB_H_)

#define _BMLIB_H_ 

#include "bmerr.h"

#define BM_TAG_ARRAY_SIZE 16

//typedef unsigned long BMIdent ;
typedef unsigned char * BMIdent ;

typedef struct {
  int active,
    version,  /** Numero de version */
    element_data_size,     /** Size of 1 element*/
    max_elements;   /** number maximum of elements */
  int nb_elements ;   /** Actul number of elements */
  int owner ;    /** PID of owner process */
} BM_STATUS ;

typedef struct {
  double sec_min, sec_max ;
} BM_SEARCH_KEY ;

/*******************************/
char * BMVersion(void);
void BMSetKey( int second, int micros, int ref, int delta,
	       BM_SEARCH_KEY * key ) ;
int BMSetVerbose( int new_verbose ) ;
int BMDumpHeader(BMIdent bmid, FILE *fout, int full );
BMIdent BMCreate(int *bmid, char *name, int element_size, int nb_elements,
		 unsigned int * key );
BMIdent BMLink(int * bmid, char *name, int element_size, int nb_elements,
	       unsigned int * key );
int BMUnlink(BMIdent bmaddr, int bmid);
int BMReset(BMIdent bmid);
int BMGetStatus(BMIdent bmid, BM_STATUS *pstat);
int BMAdd(BMIdent bmid, void *data, unsigned char *tag);
int BMFindNext(BMIdent bmid, void *data,
	       int (*compar)( const void *, const void *),
	       BM_SEARCH_KEY * key, void **next);
int BMFind(BMIdent bmid, void *data,
	   int (*compar)( const void *, const void *),
	   BM_SEARCH_KEY * key, void **next);
int BMFindTag(BMIdent bmid, void *data,
	      int (*compar)( const void *, const void *),
	      BM_SEARCH_KEY * key, int tagidx, u_char oldtag, u_char newtag);
int BMFindOldTag(BMIdent bmid, void *data, int tagidx,
		 u_char oldtag, u_char newtag);
int BMFindNewTag(BMIdent bmid, void *data, int tagidx,
		 u_char oldtag, u_char newtag);
int BMFindOld(BMIdent bmid, void *data);
int BMFindNew(BMIdent bmid, void *data);
int BMGetOldList(BMIdent bmid, int (*handle)(const void *));
int BMGetNewList(BMIdent bmid, int (*handle)(const void *));
char * BMErrorStr( int error ) ;

#endif
