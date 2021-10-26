#include "rs_debug.h"
struct rs_debug rs_debug_init(char *filename,char *mode)
{
  struct rs_debug x;
  x.arq=fopen(filename,mode);
  return(x)
}
void rs_debug_write(struct rs_debug *debug,unsigned char *buff,int nd,int from_ub)
{
  gettimeofday(&(debug->tt),NULL);
  debug->str.sec =debug->tt.tv_sec;
  debug->str.usec=debug->tt.tv_usec;
  debug->str.nd=nd;
  debug->str.dir=from_ub;
  if(x.arq!=NULL){
    fwrite(&(debug->str.sec),sizeof(uint32_t),4,debug->arq);
    fwrite(buff,sizeof(unsigned char),nd,debug->arq);
    fflush(debug->arq);
  }
}

int rs_debug_read(struct rs_debug *debug,unsigned char *buff)
{
  int nd;
  if(debug->arq!=NULL) {
    nd=fread(&(debug->str),sizeof(debug->str),1,debug->arq);  
    if(nd!=4){
      return(-1);
    }
    fread(buff,1,debug->str.nd,debug->arq);
  }
}
