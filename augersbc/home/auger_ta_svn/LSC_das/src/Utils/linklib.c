#include <stdio.h>
#include <stdlib.h>

/**
 * @defgroup linklib  Linked List library
 * @ingroup services_libraries
 *
 */
/**@{*/

#include "linkdefs.h"
#include "linklib.h"

void * LinkCreate( int size )
{
  LINK_STRUCT *link ;
  void * new ;

  size += sizeof( LINK_STRUCT ) ;
  if ( !(link = malloc( size )) ) return NULL ;
  link->prev = link->next = NULL ;
  link->tail = link->head = NULL ;
  new = GET_ITEM( link ) ;

  return new ;
}

int LinkDelete( void *old )
{
  LINK_STRUCT *iold ;

  if ( !old ) return 0 ;
  GET_LINK( iold, old ) ;
  if ( iold->prev || iold->next || iold->tail || iold->head )
    LinkUnlink( old ) ;
  free( iold ) ;
  return 1 ;
}

void * LinkNext( void *one )
{
  LINK_STRUCT *ione, *inext ;
  if ( !one ) return NULL ;
  GET_LINK( ione, one ) ;
  inext = ione->next ;
  if ( !inext ) return NULL ;
  else return GET_ITEM( inext ) ;
}

void * LinkPrev( void *one )
{
  LINK_STRUCT *ione, *iprev ;
  if ( !one ) return NULL ;
  GET_LINK( ione, one ) ;
  iprev = ione->prev ;
  if ( !iprev ) return NULL ;
  else return GET_ITEM( iprev ) ;
}

void * LinkAddHead( void *new, void **head, void **tail )
{
  LINK_STRUCT *inew, *iold ;

  /* Add at the BEGINNING of the list */
  if ( !new ) return NULL ;
  GET_LINK( inew, new ) ;
  if ( !*head ) {
    /* Empty list */
    *tail = new ;
    *head = new ;
    inew->prev = NULL ;
    inew->next = NULL ;
  }
  else {
    GET_LINK( iold, *head ) ;
    inew->next = iold ;
    inew->prev = NULL ;
    iold->prev = inew ;
    *head = new ;
  }
  inew->tail = tail ;
  inew->head = head ;
  return new ;
}

void * LinkAddTail( void *new, void **head, void **tail )
{
  LINK_STRUCT *inew, *iold ;

  /* Add at the END of the list */
  if ( !new ) return NULL ;
  GET_LINK( inew, new ) ;

  if ( !*tail ) {
    /* Empty list */
    *tail = *head = new ;
    inew->prev = inew->next = NULL ;
  }
  else {
    GET_LINK( iold, *tail ) ;
    inew->prev = iold ;
    inew->next = NULL ;
    iold->next = inew ;
    *tail = new ;
  }
  inew->tail = tail ;
  inew->head = head ;

  return (void *)new ;
}

void * LinkAfter( void *new, void *old )
{
  LINK_STRUCT *inew, *iold ;

  if ( !new ) return NULL ;
  GET_LINK( inew, new ) ;
  if ( inew->head || inew->tail ) return NULL ; /* ERROR */
  if ( !old ) return NULL ;
  GET_LINK( iold, old ) ;
  if ( !iold->head || !iold->tail ) return NULL ;
  if ( iold->next ) iold->next->prev = inew ;
  else *(iold->tail) = new ;
  inew->next = iold->next ;
  inew->prev = iold ;
  iold->next = inew ;
  inew->head = iold->head ;
  inew->tail = iold->tail ;
  return new ;
}

void * LinkBefore( void *new, void *old )
{
  LINK_STRUCT *inew, *iold ;
  if ( !new ) return NULL ;
  GET_LINK( inew, new ) ;
  if ( inew->head || inew->tail ) return NULL ; /* ERROR */
  if ( !old ) return NULL ;
  GET_LINK( iold, old ) ;
  if ( !iold->head || !iold->tail ) return NULL ;
  if ( iold->prev ) iold->prev->next = inew ;
  else *(iold->head) = new ;
  inew->next = iold ;
  inew->prev = iold->prev ;
  iold->prev = inew ;
  inew->head = iold->head ;
  inew->tail = iold->tail ;
  return new ;
}

void * LinkUnlink( void *old )
{
  LINK_STRUCT *iold ;

  if ( !old ) return NULL ;
  GET_LINK( iold, old ) ;
  if ( !iold->head || !iold->tail ) return NULL ; /* ERROR, Not linked */
  if ( iold->next ) iold->next->prev = iold->prev ;
  else if ( iold->prev ) *(iold->tail) = GET_ITEM( iold->prev ) ;
  else *(iold->tail) = NULL ;

  if ( iold->prev ) iold->prev->next = iold->next ;
  else if ( iold->next ) *(iold->head) = GET_ITEM( iold->next ) ;
  else *(iold->head) = NULL ;
  iold->head = iold->tail = NULL ;
  iold->next = iold->prev = NULL ;
  return old ;
}

void * LinkFind( void *first, void *last, int (*func)() )
{
  LINK_STRUCT *cur, *ptr ;
  LINK_STRUCT *begin, *end ;

  if ( !first ) return NULL ;
  GET_LINK( begin, first ) ;
  GET_LINK( end, last ) ;
  for ( cur = begin ; cur ; cur = cur->next ) {
    ptr = (void *)((unsigned int)cur + sizeof( LINK_STRUCT )) ;
    if ( func( ptr ) == 0 )
      return GET_ITEM( cur ) ;
  }
  return NULL ;
}

int LinkSwap( void *one, void *two )
{
  LINK_STRUCT *lone, *ltwo ;
  void *pone, *ptwo ;

  if ( !one || !two ) return -1 ;
  GET_LINK( lone, one ) ;
  GET_LINK( ltwo, two ) ;

  if ( !lone->head || !lone->tail ) return -1 ;
  if ( !ltwo->head || !ltwo->tail ) return -1 ;
  pone = LinkCreate( 16 ) ;
  ptwo = LinkCreate( 16 ) ;
  /* ajouter pone apres one*/
  /* ajouter ptwo apres two */
  LinkAfter( pone, one ) ;
  LinkAfter( ptwo, two ) ;
  /* unlinker one */
  /* unlinker two */
  LinkUnlink( one ) ;
  LinkUnlink( two ) ;
  /* ajouter two avant pone */
  /* ajouter one avant two */
  LinkBefore( two, pone ) ;
  LinkBefore( one, ptwo ) ;
  /* unlinker pone et ptwo */
  LinkUnlink( pone ) ;
  LinkUnlink( ptwo ) ;
  LinkDelete( pone ) ;
  LinkDelete( ptwo ) ;

  return 0 ;
}

/**@}*/
