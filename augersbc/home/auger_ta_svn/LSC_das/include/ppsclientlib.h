#if !defined(_PPSCLIENTLIB_H)

#define _PPSCLIENTLIB_H

#define PPSIRQ_QUEUE_NAME "ppsirq"

#define PPS_CLIENT_OPEN_ACTION 0
#define PPS_CLIENT_CLOSE_ACTION 1

typedef struct {
  int action ;
  char name[32] ;
} PPS_CLIENT_HEADER ;

typedef struct {
  PPS_CLIENT_HEADER header ;
} PPS_CLIENT_REGISTER_MSG ;

int PpsClientRegister( char * my_queue ) ;
int PpsClientUnRegister( char * my_queue ) ;

#if 0

#define PPS_LINUX_SEMAPHORE_OPEN_ACTION 2
#define PPS_SEMAPHORE_CLOSE_ACTION 3

#define PPS_XENOMAI_SEMAPHORE_OPEN_ACTION 4

#define PPS_SIGNAL_OPEN_ACTION 5
#define PPS_SIGNAL_CLOSE_ACTION 6

#endif

#endif
