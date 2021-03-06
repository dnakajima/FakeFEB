///////////////////////////////////////////////////////////////////////////
//    HandleTCPClient.c
//  (For FakeCluster)
//  receives trigger signals from LOOPBACK UDP 
//  sends data through TCP
//
//
///////////////////////////////////////////////////////////////////////
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */

#define HEADERSIZE 2

#define IPADDRSIZE 4

#define EVTNOSIZE  4
#define TRGNOSIZE  4
#define TRAILRSIZE 5

#define DATAHEADERSIZE 8
#define DATAHEADERPOS 24

#include <string.h> // for memset
#include <time.h>

//for UDP connection, probably neccessary
#include <sys/types.h>
#include <netdb.h>
#include "Practical.h"

#include <arpa/inet.h>//for inet_addr

#include <stdbool.h>

void HandleTCPClient(int clntSocket, char *trigAcptPort,char *dataServAddr, int rddepth, bool PrintStatus)
{

  /***************************************/
  //  Skeleton of the message to send
  /***************************************/
  static int SendBufSize;   /* Size of sending buffer */
  //  SendBufSize=16*(rddepth*2+2);
  SendBufSize=16*(rddepth*2+4); 
  unsigned char sndBuffer[SendBufSize];/* Buffer for data string to be sent */
  
  //padding all
  char cNumber[10];
  int i;

  for(i=0;i<10;i++)
  {
    sprintf(&cNumber[i],"%d",i);
  }

  for(i=0;i<DATAHEADERPOS+DATAHEADERSIZE;i++)
  {
    memcpy(sndBuffer+i,"\x09",1);
    //offset+=10;
  }

  //*** 0 to 1 *** "FF"
  for(i=0;i<HEADERSIZE;i++)
    sndBuffer[i]=0xAA;

  for(i=DATAHEADERPOS;i<DATAHEADERPOS+DATAHEADERSIZE;i++)
    sndBuffer[i]=0xDD;

  for(i=DATAHEADERPOS+DATAHEADERSIZE;i<sizeof(sndBuffer);i++)
  {
    memcpy(sndBuffer+i,&cNumber[i%10],1);
    //offset+=10;
  }

  //*** 2 to 5 *** Server IPAddress
  struct sockaddr_in ServAddr;
  memset(&ServAddr, 0, sizeof(ServAddr));
  ServAddr.sin_addr.s_addr = inet_addr(dataServAddr);
  //printf("%x\n",ServAddr.sin_addr.s_addr);
  memcpy(&sndBuffer[2],(void*)&ServAddr.sin_addr.s_addr,IPADDRSIZE);
  
//  //*** 6 to 9 ***
//  unsigned int EvtNo = 0;
//  int evtoffset = HEADERSIZE + IPADDRSIZE;
//  //*** 10 to 13 ***
//  /* unsigned int TrgNo = 0; */
//  int trgoffset = evtoffset + TRGNOSIZE;
//  //*** 970 to 974 ***
//  memcpy(sndBuffer+970,"EEEEE",TRAILRSIZE);
//  //*** 975 ***
//  memcpy(sndBuffer+975,"\n",1);
// 

  unsigned int EvtNo = 0;
  /* int evtoffset = 16*(rddepth*2 +1); */
  /* int trgoffset = 16*(rddepth*2 +1)+4; */

  int evtoffset = 8;
  int trgoffset = 12;

  /***************************************/
  //  Signal receiver port(SERVER) 
  /***************************************/
  // Definition
  if(PrintStatus==1)
    printf("FakeCluster1 : Trigger receive port is set to # %s\n",trigAcptPort);
  //tell the system what kinds of address we want
  //**************
  // ai_family  
  //  AF_UNSPEC :Any address family
  //  AF_INET   :IPv4 address family
  //  
  // ai_flags
  //  AI_PASSIVE    :Accept on any address/port */ 
  //  AI_NUMERICHOST:Accept only on numeric address/port
  //  
  //  
  //**************
  struct addrinfo addrCriteria;                     // Criteria for address
  memset(&addrCriteria, 0, sizeof(addrCriteria));   // Zero out structure
  addrCriteria.ai_family   = AF_INET;               // IPv4 address family
  addrCriteria.ai_flags    = AI_PASSIVE;            // Accept on any address/port */
  addrCriteria.ai_socktype = SOCK_DGRAM;            // Only datagram socket
  addrCriteria.ai_protocol = IPPROTO_UDP;           // Only UDP socket
  /***GETADDRINFO***/
  struct addrinfo *servAddr;                        // List of server addresses
  int rtnVal = getaddrinfo(NULL, trigAcptPort, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));
  /***SOCKET***/
  int trigSock = socket(servAddr->ai_family, servAddr->ai_socktype,
      servAddr->ai_protocol);
  if (trigSock < 0)
    DieWithSystemMessage("socket() failed");

  int yes =1;
  if(setsockopt(trigSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes,
		sizeof(yes))<0)
    DieWithSystemMessage("setsockopt() failed");
  /***BIND***/
  if (bind(trigSock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    DieWithSystemMessage("bind() failed");
  
  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);


  /***************************************/
  //  Handling process
  /***************************************/
  for(;;)
    {
      memcpy(sndBuffer+evtoffset,&EvtNo,sizeof(unsigned int));
      EvtNo++;
      /***preparation for trigger acception***/
      // Client address
      struct sockaddr_storage clntAddr; 
      // Set Length of client address structure (in-out parameter)
      socklen_t clntAddrLen = sizeof(clntAddr);
      // Size of trigger signal message
      char buffer[MAXSTRINGLENGTH]; // I/O buffer

      /***trigger acception***/
      // (Block until receive message from a client)
      ssize_t numBytesRcvd = recvfrom(trigSock, buffer, sizeof(unsigned int), 0,
				      (struct sockaddr *) &clntAddr, &clntAddrLen);
      if (numBytesRcvd < 0)
	DieWithSystemMessage("recvfrom() failed");

      memcpy(sndBuffer+trgoffset,buffer,numBytesRcvd);
      /***trigger acception message***/
      if(PrintStatus)
	{
	  fputs("Trigger signal is accepted from ", stdout);
	  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
	  fputc('\n', stdout);
	}
      /***sending data***/   
      if(send(clntSocket, sndBuffer, sizeof(sndBuffer), 0)<0)
	break;//DieWithError("Send error \n");
    }
  /* Close client socket */
  close(trigSock);      
  close(clntSocket);    
}
