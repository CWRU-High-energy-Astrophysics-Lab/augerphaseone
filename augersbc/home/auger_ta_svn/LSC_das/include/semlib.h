/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-02-11 17:20:02 #$
  $Revision:: 660              $

********************************************/

#if !defined(_SEMLIB_H)

#define _SEMLIB_H
/*
  In principle semflag are made from SEM_R and SEM_A.
  For some mysterious reason SEM_R and SEM_A are not defined.
  For Read/Alter for everybody, semflag should be
   SEM_R | SEM_A | (SEM_R>>3) | (SEM_A>>3) | (SEM_R>>6) | (SEM_A>>6)
*/
#define SEM_R_U 0x100
#define SEM_A_U 0x80
#define SEM_R_G 0x20
#define SEM_A_G 0x10
#define SEM_R_O 0x4
#define SEM_A_O 0x2

typedef union semun {
  int                 val;
  struct   semid_ds * buffer;
  unsigned short int * array ;
  struct seminfo *__buf;    /* buffer for IPC_INFO */
} semun_t;

int SemLink( char * name, unsigned int * key ) ;
int SemAttach( char * name, unsigned int * key ) ;
int SemDetach( int the_id ) ;
int SemP( int the_id ) ;
int SemV( int the_id ) ;
int SemSet( int the_id, unsigned int value ) ;
int SemStatus( int the_id, struct semid_ds * status ) ;
int SemValue( int the_id, int * value ) ;

#endif
