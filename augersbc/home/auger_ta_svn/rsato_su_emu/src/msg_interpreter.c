#include "msg_interpreter.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* reserv(4),Id(2),#mess(1),#msg1,msg1(n1),#msg2,msg2(n2),... */

int cdas_emu_init(struct cdas_emu_str *str)
{
  int i;
  str->nmsgs=0;
  str->st_id=0;
  for(i=0;i<5;i++){
    str->slice[i].msg=(unsigned char *)malloc(MAX_SLICE_SIZE);
    str->slice[i].last_update=0;
    str->slice[i].nb=0;
    if(str->slice[i].msg==NULL){
      printf("problem in allocate the memory for the slices ...\n");
      exit(0);
    }
  }
  T2_init(&(str->t2s),NT2SECS);
  str->nt3_to_req=0;
  t3req_init(&(str->t3req),"T3Req"); /*T3Req just store a number to make the event uniq */


  str->t2out=fopen("T2_list.out","aw");/*store (appending) the T2 list */
  setvbuf(str->t2out,(char *)NULL,_IOLBF,0);/*Line buff, flush new lines */
  str->t3out=fopen("T3.out","aw"); /*store (appending) the T3 list ...*/
}

int cdas_emu_add(struct cdas_emu_str *str,unsigned char *dt,int ndt)
{
  if(ndt<=150){
    str->ndt=ndt-7;
    memcpy(str->msg ,dt+7, str->ndt);
    str->pt=0;
    str->st_id=dt[4]*256+dt[5];
    str->nmsgs=dt[6];
  } else {
    printf("msg size is too big %d\n",ndt);
    str->nmsgs=0;
  }
  return(0);
}

int cdas_emu_get(struct cdas_emu_str *str,unsigned char *dt)
{
  int nd;
  if (str->pt<str->ndt && str->nmsgs>0){
    nd=str->msg[str->pt]+1; /*the number of bytes in the message normally do not
			      include the lenght itself, the message
			      output message will include, therefore it
			      need to be considered */
    if(str->pt + nd  <= str->ndt){
      memcpy(dt,str->msg+str->pt,nd); 
      str->pt+=nd;
      str->nmsgs--;
    } else {
      printf("cdas_emu_get: Something is wrong ... %d %d %d\n",str->pt+nd,str->ndt,str->nmsgs);
      nd=0;
      str->nmsgs=0;
    }
  } else 
    nd=0;
  return(nd);
}


/* the following functions would manipulate the slices ... */

int cdas_emu_slice_clean(struct cdas_emu_slice_str *sl)
{
  sl->last_update=0;
  sl->slice=0;
  sl->nb=0;
}
int cdas_emu_slice_add(struct cdas_emu_slice_str *sl,unsigned char *mess,int nb)
{
  int i;
  if( (sl->nb + nb) < MAX_SLICE_SIZE){
    memcpy(sl->msg + sl->nb,mess,nb);
    sl->nb+=nb;
    sl->slice=(sl->slice+1) & 0x3F;
    return(0);
  } 
  return(1);
}

int  cdas_emu_slice_first(struct cdas_emu_slice_str *sl,
		       int slice,int type,int mess_id,
		       int version,unsigned char *dt,int nb)
{
  printf("First slice ...\n");
  sl->last_update=1;
  if(nb<0 || MAX_SLICE_SIZE<=nb){
    return(1);
  }
  memcpy(sl->msg,dt,nb);
  sl->nb=nb;
  sl->type=type;
  sl->mess_id=mess_id;
  sl->version=version;
  sl->slice=(slice+1)&0x3F;
  sl->last_update=1;

  printf("type,mess_id: %d %d\n",sl->type,sl->mess_id);
  return(0);
}


int  cdas_emu_slice_last(struct cdas_emu_slice_str *sl,
			 unsigned char *dt,int nb,
			 struct cdas_emu_slice_str *out)
{
  printf("last slice ...\n");
  if(cdas_emu_slice_add(sl,dt,nb)!=0){
    cdas_emu_slice_clean(sl);
    return(1);
  }
  memcpy(out->msg,sl->msg,sl->nb);
  out->type    =sl->type;
  out->version =sl->version;
  out->nb      =sl->nb;
  cdas_emu_slice_clean(sl);
  return(0);
}


int cdas_emu_get_msg(struct cdas_emu_str *str,
		     unsigned char *msg_in,
		     struct cdas_emu_slice_str *msg_out)
{
  /*
    str-> main struct control for cdas interpreter ...
    msg_in -> data of the message, it is not need to include the
    .     length, because the first byte contain the length
    msg_out -> if there are some available message, it
    .     will store the information in this structure

    return 1   if there is an available message, it
    .            would return with *msg_out 
    .      0   if it is Ok, but there are not additional message 
    .      -1  in case of some internal error ...
  */

    
  int nb,i;
  int compl,slice,type,mess_id,vers;
  int found;
  nb=msg_in[0];
  compl=(msg_in[1]>>6)&0x3; /*it is only the 2 most significant bits */
  slice=msg_in[1] & 0x3F; 
  type=msg_in[2];
  mess_id=msg_in[3]>>2;
  vers=msg_in[3] & 0x03;

  /* compl is actually completion */
  printf("compl: %d\n",compl);
  switch(compl){
  case 0: /*the message is of only one package */
    if(nb>3){
      memcpy(msg_out->msg,msg_in+4,nb-3);
    }
    msg_out->slice=0;
    msg_out->type=type;
    msg_out->mess_id=mess_id;
    msg_out->version=vers;
    msg_out->nb=nb-3;
    return(1);
    break;
  case 2: /* additional data of previously transmitting message (next)*/
    for(i=0,found=-1;  found==-1 && i<5;  i++){
      if(str->slice[i].last_update!=0 && 
	 str->slice[i].mess_id==mess_id &&
	 str->slice[i].type==type)
	found=i;
    }
    if (found!=-1){
      printf("Found message ...%d %d\n",found,slice);
      i=found;
      if(str->slice[i].nb+nb-3 <MAX_SLICE_SIZE &&
	 str->slice[i].slice == slice){
	cdas_emu_slice_add(&(str->slice[i]),
			   msg_in+4,
			   nb-3);
	return(0);
      } else {
	printf("Not possible to handle the message. It looks too big. "
	       "Size=%d %d\n",str->slice[i].nb+nb-3);
	printf("slices ...: %d %d\n",str->slice[i].slice,slice);
	cdas_emu_slice_clean(&(str->slice[i]));
      }
    } else {
      printf("message not found ...\n");
    }
    break;
  case 1: /* the message may have more than one message. This is the first one */
    for(i=0,found=-1;  found==-1 && i<5;  i++){
      printf("case 1: %d %d %d\n",found,str->slice[i].last_update,i);
      if(str->slice[i].last_update==0){
	found=i;
      }
    }
    if(found!=-1){
      i=found;
      printf("slice,type,mess_id,vers,nb: %d %d %d %d %d\n",
	     slice,type,mess_id,vers,nb);
      cdas_emu_slice_first(&(str->slice[i]),
			   slice,type,mess_id,vers,msg_in+4,nb-3);
      
    }
    break;
  case 3: /* this is the last package of multiple slice message */
    printf("message Id=%d type=%d\n",mess_id,type);
    for(i=0,found=-1;  found==-1 && i<5;  i++){
      printf("xx: %d %d\n",str->slice[i].mess_id,str->slice[i].type);
      if(str->slice[i].last_update!=0 &&
	 str->slice[i].mess_id==mess_id &&
	 str->slice[i].type==type)
	found=i;
    }
    if (found!=-1){
      i=found;
      if(str->slice[i].nb+nb-3 <MAX_SLICE_SIZE &&
	 str->slice[i].slice == slice){
	cdas_emu_slice_last(&(str->slice[i]),
			   msg_in+4,nb-3,msg_out);
	return(1);
      } else {
	cdas_emu_slice_clean(&(str->slice[i]));
      }
    } else {
      printf("message not found ...\n");
    }
    break;
  }
  return(0);
  
}
