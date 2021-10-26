#include "t3req.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int t3req_init(struct t3req_str *t3req,char *filename)
{
  FILE *arq;
  int aux;
  
  t3req->filename=malloc(strlen(filename)+1);
  strcpy(t3req->filename,filename);
  arq=fopen(filename,"r");
  if(arq==NULL){
    printf("not possible to open %s \n",filename);
    t3req->req_id=0;
    t3req->evid=0;
  } else {
    fscanf(arq,"%d %d",&(t3req->evid),&(aux));
    t3req->evid+=10;
    t3req->req_id=(aux+10)&0xFFFF;
    fclose(arq);
  }
  return(0);
}

int t3req_generic(struct t3req_str *t3req,
		  int second,
		  int usec,
		  int window,
		  char *msg)
{
  FILE *arq;

  usec+=125;

  msg[0]=13;
  msg[1]='t';

  msg[2]=(t3req->evid>>8)&0xFF;
  msg[3]=(t3req->evid)&0xFF;

  msg[4]=(second>>24) & 0xFF;
  msg[5]=(second>>16) & 0xFF;
  msg[6]=(second>>8) & 0xFF;
  msg[7]=(second) & 0xFF;

  msg[8]= (usec>>24) & 0xFF;
  msg[9]= (usec>>16) & 0xFF;
  msg[10]=(usec>>8) & 0xFF;
  msg[11]=(usec) & 0xFF;     

  msg[12]=-125; /*offset */
  msg[13]=window&0xFF;

  t3req->evid=(t3req->evid + 1) & 0xFFFF;
  t3req->req_id++;
  if(t3req->req_id % 10 == 0){
    arq=fopen(t3req->filename,"w");
    if(arq!=NULL){
      fprintf(arq,"%d %d\n",t3req->req_id,t3req->evid);
      fclose(arq);
    } else {
      printf("Not possible to open %s file \n",t3req->filename);
    }
  }
}

int t3req_random(struct t3req_str *t3req,struct t2list *t2,char *msg)
{
  int i;
  int usec;
  i=rand()%t2->nt2;
  
  if (0<=i && i<t2->nt2)
    usec=t2->t2[i].usec;
  else
    usec=t2->t2[0].usec;
  
  t3req_generic(t3req,t2->second,usec,0,msg);
  return(0);  
}
