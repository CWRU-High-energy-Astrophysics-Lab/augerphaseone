#if !defined(_SHMLIB_H_)

#define _SHMLIB_H_

#define SHARED_MEM_INITIALIZED 0xBABEFACE

void * ShmCreate( char * name, int size, int * shmid, unsigned int * key ) ;
void * ShmLink( char * name, int size, int * shmid, unsigned int * key ) ;
int ShmDetach( void * mem ) ;
int ShmRemove( void * mem, int shmid ) ;

/* Obsolescent ... */
int ShmBind( char * name, int size, int * shmid, unsigned int * key ) ;
void * ShmAttach( char * name, int size, int * shmid, unsigned int * key ) ;

#endif
