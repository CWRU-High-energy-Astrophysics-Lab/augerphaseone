#include <sys/time.h>

struct debug_str
{
  struct timeval tt;
  uint32_t dir;
  uint32_t nd;
  unsigned char buff[32000];
};
struct rs_debug
{
  FILE *arq;
  struct debug_str str;
};
struct rs_debug rs_debug_init(char *filename);
void rs_debug_write(unsigned char *buff,int nd,int from_ub);
