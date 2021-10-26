/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-03-14 12:41:35 #$
  $Revision:: 805              $

********************************************/

/*

  History

  V1 - guglielm - 2010/10/06
    Creation

*/

#if !defined(_MBLIB_H)

#define _MBLIB_H

/*
  Only one producer and 1 consumer
  An event has 3 possible states:
    free: can be allocated to the producer.
    used: currently in use by the producer.
    ready: can be used by the consumer.

  free -----------> used ------------> ready -------------> free
         MbAlloc            MbReady              MbGet

*/
typedef enum {
  MB_OK,
  MB_FIRST_ERROR = 9100,
  MB_CANT_CREATE = MB_FIRST_ERROR,
  MB_CANT_LINK,
  MB_NOT_INITIALIZED,
  MB_CANT_CREATE_SEMAPHORE,
  MB_NO_FREE_ELEMENTS,		/**< For the producer */
  MB_NO_READY_ELEMENTS,		/**< For the consumer */
  MB_LAST_ERROR = MB_NO_READY_ELEMENTS
} MB_ERROR_CODE ;
  
typedef struct {
  int mb_id ;
  /* Lock Semaphore */
  unsigned int mb_lock_key ;
  int mb_lock ;

  unsigned int mb_key ;
  int mb_sem ;
  int sem_key ;
  int element_size ;
  int nb_elements ;		/**< Maximum Nb of MUON buffers */
  int used_elements ;
  int ready_elements ;
  int free_elements ;
  int old_element ;
  int next_element ;
} mb_status_t ;

int MblibVersionNb() ;
char * MblibVersion() ;
void MblibSetVerbose( int level ) ;

char * MbError( int err ) ;
int MbStatus( mb_status_t * status ) ;
int MbCreate( int nbelem ) ;
int MbLink( void ) ;
int MbReset( void ) ;
int MbUnlink( void ) ;
int MbSem( int * sem ) ;
int MbAlloc( void ** addr ) ;
int MbReady( void ) ;
int MbGet( void ** addr ) ;
int MbFree( void ) ;

#endif
