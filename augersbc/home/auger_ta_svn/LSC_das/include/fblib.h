/*******************************************

  $Author:: guglielmi          $
  $Date:: 2010-11-09 11:27:08 #$
  $Revision:: 203              $

********************************************/

/*

  History

  V1 - guglielm - 2010/10/06
    Creation

*/

#if !defined(_FBLIB_H)

#define _FBLIB_H

/*
  Only one producer and 1 consumer
  An event has 3 possible states:
    free: can be allocated to the producer.
    used: currently in use by the producer.
    ready: can be used by the consumer.

  free -----------> used ------------> ready -------------> free
         FbAlloc            FbReady              FbGet

*/
typedef enum {
  FB_OK,
  FB_FIRST_ERROR = 9000,
  FB_CANT_CREATE = FB_FIRST_ERROR,
  FB_CANT_LINK,
  FB_NOT_INITIALIZED,
  FB_CANT_CREATE_SEMAPHORE,
  FB_NO_FREE_ELEMENTS,		/**< For the producer */
  FB_NO_READY_ELEMENTS,		/**< For the consumer */
  FB_LAST_ERROR = FB_NO_READY_ELEMENTS
} FB_ERROR_CODE ;
  
typedef struct {
  int fb_id ;
  /* Lock Semaphore */
  unsigned int fb_lock_key ;
  int fb_lock ;

  unsigned int fb_key ;
  int fb_sem ;
  int sem_key ;
  int element_size ;
  int nb_elements ;		/**< Maximum Nb of FAST buffers */
  int used_elements ;
  int ready_elements ;
  int free_elements ;
  int old_element ;
  int next_element ;
} fb_status_t ;

int FblibVersionNb() ;
char * FblibVersion() ;
void FblibSetVerbose( int level ) ;

char * FbError( int err ) ;
int FbStatus( fb_status_t * status ) ;
int FbCreate( int nbelem ) ;
int FbLink() ;
int FbReset() ;
int FbUnlink() ;
int FbSem( int * sem ) ;
int FbAlloc( void ** addr ) ;
int FbReady() ;
int FbGet( void ** addr ) ;
int FbFree() ;

#endif
