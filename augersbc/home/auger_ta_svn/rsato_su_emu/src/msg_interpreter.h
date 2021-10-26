
#define MAX_SLICE_SIZE 102400


#include "t2.h"
#include "t3req.h"
#define NT2SECS 20

#include <stdio.h>

struct cdas_emu_slice_str
{
  int last_update;
  unsigned char *msg; /*it would be really enough ... */
  int slice;
  int type,mess_id,version;

  int nb;
};

struct cdas_emu_str
{
  unsigned char msg[150];
  int ndt;
  int pt;
  int nmsgs;
  int st_id;
  struct cdas_emu_slice_str slice[5];

  struct t2_all t2s;
  
  int nt3_to_req;
  struct t3req_str t3req;  

  FILE *t2out,*t3out;
};


int cdas_emu_init(struct cdas_emu_str *str);
int cdas_emu_add(struct cdas_emu_str *str,unsigned char *dt,int ndt);
int cdas_emu_get(struct cdas_emu_str *str,unsigned char *dt);
int cdas_emu_get_msg(struct cdas_emu_str *str,
		     unsigned char *msg_in,
		     struct cdas_emu_slice_str *msg_out);

