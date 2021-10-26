#if !defined(_LINKLIB_H)

#define _LINKLIB_H

void *LinkCreate( int size ) ;
int LinkDelete( void *old ) ;
int LinkSwap( void *one, void *two ) ;
void *LinkNext( void *one ) ;
void *LinkPrev( void *one ) ;
void *LinkAddHead( void *new, void **head, void **tail ) ;
void *LinkAddTail( void *new, void **head, void **tail ) ;
void *LinkAfter( void *new, void *old ) ;
void *LinkBefore( void *new, void *old ) ;
void *LinkUnlink( void *old ) ;
void *LinkFind( void *first, void *last, int (*func)() ) ;

#endif
