//////////////////////////////////////////////////////////////////
//   BroadCastRelay.c
//   By Kazuma Ishio.
//   Last update : 2014/08/16
//   This program intermediates TriggerSender and FakeCluster(s).
//   
//   [Flow of the program]
//   TriggerSender
//     |
//     | TriggerSignal(Broadcast UDP packet)
//     V
//   BroadCastRelay (as a gate of a PC)
//     | 
//     | TriggerSignals(Loopback UDP packets)
//     V
//   FakeCluster(s) (all listed processes in the PC)
//
//  [Usage]
//  compile:
//   gcc -o BroadCastRelay -std=gnu99
//      BroadCastRelay.c DieWithMessage.c  AddressUtility.c 
//  submit:
//   sudo ./BroadCastRelay <port1> <port2> ...
//  
//  [Note] 
//  TriggerSender must set "port" (destination port) 
//  to the same as 
//  "trgAcptPort" (receive port) in BroadCastRelay.
//  By default it is "22222"
//  
//  
//  
//   
//////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Practical.h"
//#include <sys/types.h>
#include <netdb.h>//addrinfo


int main(int argc,char *argv[]){
  int PrintStatus=0;
  char *trigAcptPort;  
  trigAcptPort="22222";

  /*********************************************/
  //   Broadcast Receiver socket(SERVER)
  /*********************************************/
  printf("BROADCAST RECEIVER:");
  printf("Receiver port is %s.\n",trigAcptPort); 
  struct addrinfo addrCriteria;                   // Criteria for address
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family   = AF_INET;             // Any address family
  addrCriteria.ai_flags    = AI_NUMERICHOST;      // Accept only on numeric address/port
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

  struct addrinfo *servAddr;                      // List of server addresses
  int rtnVal = getaddrinfo("192.168.10.255", trigAcptPort, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  /***SOCKET***/
  int recvsock = socket(servAddr->ai_family, servAddr->ai_socktype,
      servAddr->ai_protocol);
  if (recvsock < 0)
    DieWithSystemMessage("socket() failed");

  /***BIND***/
  if (bind(recvsock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    DieWithSystemMessage("bind() failed");
  
  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  /*********************************************/
  //   Loopback Sender socket(CLIENT)
  /*********************************************/
  /***destination ports***/
  //info of the number of arguments to be treated
  const int nPorts=argc -1 ;
  printf("LOOPBACK SENDER:");
  printf("%d destination ports are set.\n",nPorts); 
  
  in_port_t destPort[nPorts];
  struct sockaddr_storage destStorage[nPorts];
  struct sockaddr_in *destAddr[nPorts];
  struct sockaddr *destAddress[nPorts];
  size_t addrSize = sizeof(struct sockaddr_in);
  for(int i=0;i<nPorts;i++){
    //port number preparation
    destPort[i]=atoi(argv[i+1]);
    printf("port no: %d\n",destPort[i]);
    //write struct of sockaddr_in, to destStorage
    memset(&destStorage[i], 0 , sizeof(destStorage[i]));
    destAddr[i]= (struct sockaddr_in *)&destStorage[i];
    destAddr[i]->sin_family = AF_INET;
    destAddr[i]->sin_port = htons(destPort[i]);
    destAddr[i]->sin_addr.s_addr=htonl(INADDR_LOOPBACK);//set destination IP -localhost, 127.0.0.1;
    //sockaddr
    destAddress[i] = (struct sockaddr *) &destStorage[i];        
  }
  /***departure ports***/
  struct sockaddr_in fromAddr;

  /***SOCKET***/
  int sendsock[nPorts];
  for(int i=0;i<nPorts;i++){
    sendsock[i]= socket(destAddress[i]->sa_family, SOCK_DGRAM, IPPROTO_UDP);
    if(sendsock[i]<0)
      DieWithSystemMessage("send socket() failed");    
  }

  /***************************************/
  //  Handling process
  /***************************************/
  for(;;)
    {
      /***preparation for trigger acception***/
      // Client address
      struct sockaddr_storage clntAddr; 
      // Set Length of client address structure (in-out parameter)
      socklen_t clntAddrLen = sizeof(clntAddr);
      // Size of trigger signal message
      char buffer[MAXSTRINGLENGTH]; // I/O buffer

      /***trigger acception***/
      // (Block until receive message from a client)
      ssize_t numBytesRcvd = recvfrom(recvsock, buffer, MAXSTRINGLENGTH, 0,
				      (struct sockaddr *) &clntAddr, &clntAddrLen);
      if (numBytesRcvd < 0)
	DieWithSystemMessage("recvfrom() failed");
      
      /***trigger acception message***/
      if(PrintStatus==1)
	{
	  fputs("Trigger signal is accepted from ", stdout);
	  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
	  fputc('\n', stdout);
	}
      /***sending data***/
      for(int i=0;i<nPorts;i++){
	if(sendto(sendsock[i], buffer, numBytesRcvd, 0, destAddress[i], addrSize)<0)
	  break;
      }
	//fputc('.',stderr);
    }



}





