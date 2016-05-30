////////////////////////////////////////////////////////////////////////////
// [TriggerSender.c]
//  By Kazuma Ishio
//  Last update : 2015/05/11
//  This program sends UDP broadcast packets
//   to the port you specified 
//   at the rate you specified.
//
//   Materials:
//     TriggerSender.c
//     Practical.h
//     DieWithMessage.c
//   How To Compile:
//      gcc -o TriggerSender -std=gnu99 -lrt TriggerSender.c DieWithMessage.c 
//   (TCPServerUtility.c TCPClientUtility.c AddressUtility.c are not necessary any more.)
//   How To Submit:
//     ./TriggerSender <Port> <Trigger rate[Hz]>
//     ./TriggerSender  22222  20000
//  
//   
// 
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Practical.h"

#include <sys/timerfd.h>
#include <assert.h>
//#include <time.h>
//#include <netdb.h>
//#include <netinet/in.h>

static const char *IN6ADDR_ALLNODES = "FF02::1";// v6 addr not built in

int main(int argc, char *argv[]){

  /******************************************/
  //  Handling input arguments
  /******************************************/
  if(argc != 3 && argc !=2)//Test for correct number of arguments
    DieWithUserMessage("Parameter(s)"," <Port> <Trigger rate[Hz]>");
  
  //arg1
  in_port_t port = htons((in_port_t) atoi(argv[1]));

  //former arg1 was cut out.
  struct sockaddr_storage destStorage;
  memset(&destStorage, 0, sizeof(destStorage));  
  size_t addrSize =0;
  //  if(argv[1][0]== '4'){
    struct sockaddr_in *destAddr4 = (struct sockaddr_in *) &destStorage;
    destAddr4->sin_family = AF_INET;
    destAddr4->sin_port = port;
    destAddr4->sin_addr.s_addr= inet_addr("192.168.1.255");//INADDR_BROADCAST;
    //destAddr4->sin_addr.s_addr= INADDR_BROADCAST;//This causes sendto() error "Network is unreachable"
    addrSize = sizeof(struct sockaddr_in);
  //}else if (argv[1][0] == '6'){
  //  struct sockaddr_in6 *destAddr6 = (struct sockaddr_in6 *)&destStorage;
  //  destAddr6->sin6_family = AF_INET6;
  //  destAddr6->sin6_port = port;
  //  inet_pton(AF_INET6, IN6ADDR_ALLNODES, &destAddr6->sin6_addr);
  //  addrSize = sizeof(struct sockaddr_in6);
  //}else{
  //  DieWithUserMessage("Unknown address family", argv[1]);
  //}
  struct sockaddr *destAddress= (struct sockaddr *) &destStorage;

  /* //arg3 */
  /* size_t msgLen = strlen(argv[3]); */
  /* if(msgLen > MAXSTRINGLENGTH) //Input string fits? */
  /*   DieWithUserMessage("String too long",argv[3]); */

  //arg2
  int rate = 100;/*Trigger rate 100Hz as default*/
  if(argc==3)rate=atoi(argv[2]);
  int interval = (int)(1000000000.0 / rate);
  fprintf(stderr, "trigger rate is set to %d [Hz], interval is set to %d [ns]\n",rate,interval);
  /******************************************/
  //  timerfd
  /******************************************/
  struct itimerspec its;
  int ret;
  int timerfd;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = interval;
  its.it_value.tv_sec = 1;
  its.it_value.tv_nsec = 0;

  timerfd = timerfd_create(CLOCK_MONOTONIC, 0);

  ret = timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &its, NULL);

  
  /******************************************/
  //  Preparation of a socket
  /******************************************/
  //Create socket for sending/receiving datagrams
  int sock = socket(destAddress->sa_family, SOCK_DGRAM, IPPROTO_UDP);
  if(sock < 0)
    DieWithSystemMessage("socket() failed");
  
  //Set socket to allow broadcast
  int broadcastPerm = 1;
  if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastPerm,sizeof(broadcastPerm)) < 0)
     DieWithSystemMessage("setsockopt() failed");

  /******************************************/
  //  Sending UDP messages forever
  /******************************************/
  unsigned int TrgNo=0;
  for(;;){
    //TIMER
    uint64_t v;
    ret = read(timerfd,&v,sizeof(v));
    assert(ret == sizeof(v));
    
    //Broadcast msgString in datagram to clients
    ssize_t numBytes = sendto(sock, &TrgNo,sizeof(unsigned int), 0, destAddress, addrSize);
    if(numBytes < 0)
      DieWithSystemMessage("sendto() failed");
    else if (numBytes != sizeof(unsigned int))
      DieWithUserMessage("sendto()","sent unexpected number of bytes");
    
    //TIMER EVAL
    if (v > 1) {
      //      fputc('o', stderr);
    } 
    /* else { */
    /*   fputc('.', stderr); */
    /* } */
    TrgNo++;
    
  }

  // NOT REACHED

 }
