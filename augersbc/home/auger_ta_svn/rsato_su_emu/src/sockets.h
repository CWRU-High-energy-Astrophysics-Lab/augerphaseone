
#ifndef SOCKETS
#define SOCKETS

#include <stdint.h>

int make_socket_server (char *hostname,uint16_t port);
int make_socket_client (char *hostname,uint16_t port);
int new_client(int sock);

#endif
