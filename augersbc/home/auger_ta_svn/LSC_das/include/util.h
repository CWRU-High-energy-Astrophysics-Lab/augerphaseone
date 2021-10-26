#if !defined(_UTIL_H_)
#define _UTIL_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2011-08-26 16:55:32 +0200 (Fri, 26 Aug 2011) $
  $Revision: 1454 $

********************************************/

#define TASK_INITIALIZED_SEMAPHORE "TaskReady"

typedef struct {
  char *name ;
  unsigned int key ;
  int start ;
  int has_key ;
  int wait ; /* Wait time in micros */
} TASK_ENTRY ;

enum {
  DONOT_START, DO_START
} ;

enum {
  HAS_KEY, HAS_NO_KEY
} ;

unsigned int mkkey( unsigned char * name ) ;
int is_running( char * taskname ) ;
char * get_bin_dir( char * argv ) ;

#define EXIT_CODE_FILE_NAME "exit_code"
void set_exit_code( unsigned int code ) ;
unsigned int get_exit_code() ;

void TaskReady() ;

int isPowerOn() ;

#endif
