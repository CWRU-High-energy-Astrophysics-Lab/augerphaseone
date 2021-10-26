#if !defined(_BUFLIB_H_)

#define _BUFLIB_H_ 

/**
 * @defgroup buflib_h Buffer Management Library
 * @ingroup acq_include
 *
 *
 */
/**@{*/

/**
 * @file   buflib.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Fri Feb 11 14:37:21 CET 2011
 * 
 * @brief   Buffer Management Library
 *
*/

#include "buferr.h"

#define BUF_TAG_ARRAY_SIZE 16

//typedef unsigned long BMIdent ;
typedef unsigned char * BUFIdent ;

typedef struct {
  int active,
    version,  /** Numero de version */
    element_data_size,     /**< @brief Data Size of 1 element*/
    full_element_size,		/**< @brief Element + Tag size */
    max_elements;   /**< @brief number maximum of elements */
  int nb_elements ;   /**< @brief Actual number of elements */
  int owner ;    /** PID of owner process */
  unsigned int key ;
  char name[32] ;
} BUF_STATUS ;

typedef struct {
  double sec_min, sec_max ;
} BUF_SEARCH_KEY ;

/*******************************/
char * BuflibVersion(void);
void BuflibSetKey( int second, int micros, int ref, int delta,
		   BUF_SEARCH_KEY * key ) ;
int BuflibSetVerbose( int new_verbose ) ;
int BuflibDumpHeader( BUFIdent bmid, FILE *fout, int full );
BUFIdent BuflibCreate(int *bmid, char *name, int element_size, int nb_elements,
		      unsigned int * key );
BUFIdent BuflibLink(int * bmid, char *name, int element_size, int nb_elements,
		    unsigned int * key );
int BuflibUnlink(BUFIdent bmaddr, int bmid);
int BuflibReset(BUFIdent bmid);
int BuflibGetStatus(BUFIdent bmid, BUF_STATUS *pstat);
int BuflibAdd(BUFIdent bmid, void *data, unsigned char *tag);
int BuflibFindNext( BUFIdent bmid, void *data,
		    int ( *compar)( const void *, const void *),
		    void * key, void **next);
int BuflibFind( BUFIdent bmid, void *data,
		int (*compar)( const void *, const void *),
		void * key, int * next);
int BuflibFindTag( BUFIdent bmid, void *data,
		   int (*compar)( const void *, const void *),
		   void * key, int * eidx,
		   int tagidx, u_char oldtag, u_char newtag);
int BuflibFindOldTag( BUFIdent bmid, void *data, int tagidx,
		      u_char oldtag, u_char newtag);
int BuflibFindNewTag( BUFIdent bmid, void *data, int tagidx,
		      u_char oldtag, u_char newtag);
int BuflibGetByIndex( BUFIdent bmid, void * data, int idx ) ;
int BuflibGetOld( BUFIdent bmid, void *data);
int BuflibGetNew( BUFIdent bmid, void *data);
int BuflibGetOldList( BUFIdent bmid, int (*handle)(const void *, const int ));
int BuflibGetNewList( BUFIdent bmid, int (*handle)(const void *, const int ));
int BuflibGetIndexList( BUFIdent bmid,
			int (*handle)(const void *, const int ) ) ;
int BuflibGetTagArray( BUFIdent bmid, unsigned char * tag, int idx ) ;
char * BuflibErrorStr( int error ) ;

/**@}*/

#endif
