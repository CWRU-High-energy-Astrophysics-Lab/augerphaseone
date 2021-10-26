/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-11-14 14:21:13 #$
  $Revision:: 1723             $

********************************************/

#if !defined(_CPUNUMBER_H)

#define _CPUNUMBER_H

#define DEFAULT_CPU_NUMBER 100

int CpunumberVersionNb() ;
char * CpunumberVersion() ;
void CpunumberSetVerbose( int level ) ;

int GetCpuNumber( void ) ;

#endif
