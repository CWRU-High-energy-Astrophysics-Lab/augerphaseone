/*******************************************

  $Author:: guglielmi          $
  $Date:: 2012-01-18 11:47:59 #$
  $Revision:: 1841             $

********************************************/

#if !defined(_FELIB_H)

#define _FELIB_H

/**
 * @defgroup felib_h Front End Library
 * @ingroup acq_include
 *
 */

/**@{*/
/**
 * @file   felib.h
 * @author Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>
 * @date   Fri Feb 11 14:37:21 CET 2011
 * 
 * @brief   Front End Library
*/

int FelibVersionNb() ;
char * FelibVersion() ;
void FelibSetVerbose( int level ) ;

int FelibOpen( unsigned int * id ) ;
void FelibClose() ;
int FelibHasFe( void ) ;
int FelibSetABParams( TRIGGER_TYPE trigger, int thresh, int width,
		      int occupancy ) ;
int FelibSetFromConfig( ACQ_CONFIG * acqconfig ) ;
int FelibIsEnabledTrigger( TRIGGER_TYPE trigger ) ;
int FelibEnableTriggersFromConfig( ACQ_CONFIG * acqconfig ) ;
int FelibEnableTrigger( TRIGGER_TYPE trigger ) ;
int FelibEnableTriggers( unsigned int trigger_mask ) ;
int FelibDisableAllTriggers() ;
int FelibDisableTrigger( TRIGGER_TYPE trigger ) ;
int FelibEnableMuonTrigger() ;
int FelibDisableMuonTrigger() ;

unsigned int FelibGetId( unsigned int * fe_id ) ;
int FelibSetSoftDelay( int delay ) ;
int FelibSoftTrigger() ;
int FelibSlowSoftTrigger() ;
int FelibReset() ;
unsigned int * FelibGetBase() ;
int FelibReadEvt( unsigned int * evt, unsigned int * status ) ;
int FelibSplitData( unsigned int * padc, unsigned int * anode_30,
		    unsigned int * anode_1, unsigned int * anode_01,
		    unsigned int * dynode ) ;
unsigned int FelibGetAnode_30( unsigned int * padc ) ;
unsigned int FelibGetAnode_1( unsigned int * padc ) ;
unsigned int FelibGetAnode_01( unsigned int * padc ) ;
unsigned int FelibGetDynode( unsigned int * padc ) ;
int FelibGetABParams( TRIGGER_TYPE trigger, int * thresh, int * width,
		      int * occupancy ) ;
int FelibReadMuon( unsigned int * muon ) ;

int FelibReadGrb( int * value ) ;

/**@}*/
#endif
