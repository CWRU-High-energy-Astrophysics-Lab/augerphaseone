#ifndef T3REQ
#define T3REQ

#include <stdint.h>
#include "t2.h"
struct t3req_str 
{
  char *filename;
  int req_id;    // it is used internally by the UB to identify the event
  uint32_t evid;      //general event identification.
  
};


int t3req_init(struct t3req_str *t3req,char *filename);
int t3req_generic(struct t3req_str *t3req,
		  int second,
		  int usec,
		  int window,
		  char *msg);
int t3req_random(struct t3req_str *t3req,struct t2list *t2,char *msg);


#endif
