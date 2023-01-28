// Project:      CSS432 TCP Socket Class
// Professor:    Munehiro Fukuda
// Organization: CSS, University of Washington, Bothell
// Date:         March 5, 2004
// Updated by Yang Peng on 12/10/2019

#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include <iostream>
#define MSGSIZE 1460 // TCP message size in bytes

using namespace std;

extern "C"
{
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h> // for sockets

#include <netdb.h>  // for gethostbyname( )
#include <string.h> // for bzero( )
#include <unistd.h> // for close( )

#include <sys/poll.h> // for poll( )
}

#define NULL_SD -1 // means no socket descriptor

class TcpSocket
{
public:
   TcpSocket(const char *port); // open an TCP socket with int port
   ~TcpSocket();
   bool
   setDestAddress(const char *); // set the IP addr given an IP name in char[]
   int pollRecvFrom();           // check if this socket has data to receive
   int sendTo(char[], int);      // send a message in char[] whose size is int
   int recvFrom(char[], int);    // receive a message in char[] of int size
   int ackTo(char[], int);       // send an ack message in char[] of int size

private:
   const char *port;         // this TCP port
   int sd;                   // this TCP socket descriptor
   struct sockaddr destAddr; // a destination socket address for internet
   struct sockaddr srcAddr;  // a source socket address for internet
};

#endif
