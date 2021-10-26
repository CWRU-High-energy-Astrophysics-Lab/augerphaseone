#include <stdio.h>
#include <stdint.h>


uint32_t endian(uint32_t v)
{
  unsigned char *aux,*o;
  uint32_t out;
  aux=(unsigned char *)&v;
  o=(unsigned char *)&out;
  o[3]=aux[0];
  o[2]=aux[1];
  o[1]=aux[2];
  o[0]=aux[3];
  return(out);
}
// Pass file name as argument to program. SPQ June 14 2015
main(int argc, char *argv[])
{
  unsigned char x[20000];
  uint32_t *l,aux;
  int d[768][3],a[768][3];
  int i;
  FILE *arq;
  arq=fopen(argv[1],"r");
  fread(x,20000,1,arq);
  l=(uint32_t *)(&(x[24]));
  for(i=0;i<768;i++){
    aux=endian(*l);
    d[i][0]=aux& 0x3FF;
    d[i][1]=(aux>>10)& 0x3FF;
    d[i][2]=(aux>>20)& 0x3FF;
    l++;
  }
  for(i=0;i<768;i++){
    aux=endian(*l);
    a[i][0]=aux& 0x3FF;
    a[i][1]=(aux>>10)& 0x3FF;
    a[i][2]=(aux>>20)& 0x3FF;
    l++;
  }
  FILE *f;
  f=fopen("FADC_trace", "w");//Write to file, SPQ June 14 2015
  for(i=0;i<768;i++){
    fprintf(f,"%3d  %d %d %d  %d %d %d\n",
	   i,
	   d[i][0],d[i][1],d[i][2],
	   a[i][0],a[i][1],a[i][2]);
  }
  fclose(f);
}
