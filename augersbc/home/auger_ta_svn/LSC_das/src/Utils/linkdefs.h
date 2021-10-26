#if !defined(_LINKDEFS_H)

#define _LINKDEFS_H

#define GET_LINK(x,y)	x = (void *)( (void *)y - sizeof( LINK_STRUCT ))
#define GET_ITEM(x)     (void *)( (void *)x + sizeof( LINK_STRUCT ) )

typedef struct link_s LINK_STRUCT ;

struct link_s {
  LINK_STRUCT *next, *prev ;
  void **head, **tail ;
} ;

#endif

