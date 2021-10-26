#include "t2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int T2_init(struct t2_all *t2_all,int nsecs)
{
  if(nsecs<0 || 100<nsecs){
    printf("It is too big array ...\n");
    return(1);
  }
  t2_all->nseconds=nsecs;
  t2_all->t2s=(struct t2list *)malloc(sizeof(struct t2list)*nsecs);
  if(t2_all->t2s==NULL){
    return(1);
  }
  memset(t2_all->t2s,0,sizeof(sizeof(struct t2list)*nsecs));
  t2_all->head=nsecs-1;
  return(0);
}

int T2_interpret(unsigned char *msg,int size,struct t2list *t2s)
{
  int i,type;
  unsigned char *pt;
  if(size>4){
    t2s->second=(msg[0]<<24) | (msg[1]<<16) | (msg[2]<< 8)  | msg[3];
    t2s->nt2=(size-4)/3;
    pt=msg+4;
    i=0;
    while(i<t2s->nt2){
      type=(*pt)>>4;
      if(type!=7){
	t2s->t2[i].tr_type=type;
	t2s->t2[i].usec=(((*pt)&0xF)<<16) | ((*(pt+1))<<8) | *(pt+2);
	i++;
      } else {
	t2s->scaler=(((*pt)&0xF)<<16) | ((*(pt+1))<<8) | *(pt+2);
	t2s->nt2--;
      }
      pt+=3;
    }
  } else {
    t2s->nt2=0;
  }
  return(t2s->nt2);
}

int T2_add_new(struct t2_all *t2s,unsigned char *msg,int size)
{
  int i,prev_sec,nt2s;
  struct t2list *t2;
  i=t2s->head;
  prev_sec=t2s->t2s[i].second;
  i=(i+1)%(t2s->nseconds);
  t2s->head=i;
  t2=&(t2s->t2s[i]);
  nt2s=T2_interpret(msg,size,t2);

  if(nt2s>0 && prev_sec>0 && t2->second-prev_sec>1){
    printf("missing t2s from %d %d: (%d seconds)\n",
	   prev_sec,t2->second,t2->second - prev_sec);
  }
  return(nt2s);
}


int T2_get_from_list(struct t2_all *t2s,int nsec,struct t2list **t2)
{
  /* get the T2 list from the circular buffer.
     t2s is the structure which have the circular buffer
     nsec is the number of seconds before (however it do not care if
     .    the nsec is larger than the size of circular buffer)
     t2 -> the output list. It is a pointer to the circular buffer
     .    therefore do not modify the structure in other place.
     return the number of the T2 of the list.
  */
  int i;
  i=t2s->head;
  i-=(nsec%t2s->nseconds);
  if (i<0){
    i+=t2s->nseconds;
  }
  *t2=&(t2s->t2s[i]);
  return(t2s->t2s[i].nt2);
}
