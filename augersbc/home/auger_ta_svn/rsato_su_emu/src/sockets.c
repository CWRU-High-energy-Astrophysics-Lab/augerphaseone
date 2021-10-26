#include "sockets.h"
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

init_sockaddr (struct sockaddr_in *name,const char *hostname,
	       uint16_t port)
{
  struct hostent *hostinfo;
  memset(name,0,sizeof(*name));
  name->sin_family = PF_INET;
  name->sin_port = htons (port);
  if(hostname==NULL){
    name->sin_addr.s_addr = htonl (INADDR_ANY);
  } else {
    hostinfo=gethostbyname(hostname);
    if(hostinfo==NULL){
      printf("Host not found \n");
      exit(0);
    }
    name->sin_addr=*(struct in_addr *) hostinfo->h_addr;
  }  
}



int make_socket_server (char *hostname,uint16_t port)
{
  int sock,r;
  struct sockaddr_in name;
  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)  {
    perror ("socket");
    return(-1);
  }
  init_sockaddr (&name,hostname,port);
  /* Give the socket a name. */
  r=bind(sock, (struct sockaddr *) &name, sizeof (name));
  if (r < 0) {
    perror ("bind");
    exit(0);
  }

  if(listen(sock,1)<0){
    perror("listen error");
    exit(0);
  }

  printf("socket: %d\n",sock);
  return sock;
}


int make_socket_client (char *hostname,uint16_t port)
{
  int sock;
  struct sockaddr_in name;
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0){
    perror ("socket (client)");
    return(-1);
  }
  init_sockaddr (&name,hostname,port);
  if (connect (sock,(struct sockaddr *) &name,sizeof (name))<0){
    perror ("connect (client)");
    return(-1);
  }
  return(sock);
}  


int new_client(int sock)
{
  int newfd;
  struct sockaddr clientname;
  socklen_t size;
  
  size=sizeof(clientname);
  newfd = accept(sock, &clientname, &size);
  if (newfd < 0) { 
    printf("Error while acceptiong new connection\n");
    return(-1);
  }
  return(newfd);
}
