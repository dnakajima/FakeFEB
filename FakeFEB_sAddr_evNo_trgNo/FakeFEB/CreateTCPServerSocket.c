#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define MAXPENDING 5

void DieWithError(char *errorMessage);

int CreateTCPServerSocket(char *ipaddress, unsigned short port)
{
  int sock;
  struct sockaddr_in echoServAddr;

  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    DieWithError("socket() failed");
  
  memset(&echoServAddr, 0, sizeof(echoServAddr));
  echoServAddr.sin_family = AF_INET;
  //echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  //echoServAddr.sin_addr.s_addr = inet_addr("192.168.10.9");
  echoServAddr.sin_addr.s_addr = inet_addr(ipaddress);
  echoServAddr.sin_port = htons(port);
  
  if(bind(sock, (struct sockaddr * ) &echoServAddr, sizeof(echoServAddr)) < 0)
    DieWithError("bind() failed");

  if (listen(sock, MAXPENDING) < 0)
    DieWithError("listen() failed");
  
  return sock;
}
