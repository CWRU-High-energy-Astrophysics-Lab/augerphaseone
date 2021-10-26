#if !defined(_SVRCONFIG_H_)
#define _SVRCONFIG_H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-29 10:51:30 #$
  $Revision:: 1064             $

********************************************/

#define SVR_CONFIG_FILE_NAME "SvrConfig.cfg"
#define SVR_CONFIG_DMP_NAME  "SvrConfig.dmp"

typedef struct {
  int initialized ;
  unsigned int ServicesVersion ;
  unsigned int CpuNumber ;		/**< @brief Hardware CPU Number */
  int LsId ;			/**< @brief Software LS Id (defined by CDAS */
  char LsName[16] ;		/**< @brief LS name (from locations.cfg) */
} SVR_CONFIG ;

#endif
