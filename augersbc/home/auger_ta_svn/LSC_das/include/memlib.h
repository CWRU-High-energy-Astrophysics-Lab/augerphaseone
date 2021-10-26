#if !defined(_MEMLIB_H_)

#define _MEMLIB_H_ 

typedef struct {
  int nb_blocks ;
  size_t tot_mem ;
  size_t max_mem ;
  int add_err,
    del_err,
    chg_err ;
} MEMLIB_STATUS ;

void * mem_malloc( size_t size ) ;
void * mem_calloc( size_t count, size_t size ) ;
void * mem_realloc( void * ptr, size_t size ) ;
void mem_free( void * ptr ) ;
void mem_dump( FILE * fout, int flag ) ;
size_t mem_size( void * ptr ) ;
size_t mem_status( MEMLIB_STATUS * status ) ;

#endif

