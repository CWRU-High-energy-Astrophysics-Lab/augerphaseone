#ifndef T2
#define T2
struct t2info
{
  int tr_type:8;
  int usec:24;
};

struct t2list
{
  int second;
  int nt2;
  struct t2info t2[200];
  int scaler;
};

struct t2_all
{
  int nseconds;
  int head;
  struct t2list *t2s;
};
  
int T2_init(struct t2_all *t2_all,int nsecs);
int T2_add_new(struct t2_all *t2s,unsigned char *msg,int size);
int T2_get_from_list(struct t2_all *t2s,int nsec,struct t2list **t2);

#endif
