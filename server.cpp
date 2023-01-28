#include <arpa/inet.h> // inet_ntoa
#include <errno.h>
#include <iostream>
#include <netdb.h>       // gethostbyname
#include <netinet/in.h>  // htonl, htons, inet_ntoa
#include <netinet/tcp.h> // SO_REUSEADDR
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>    // bzero
#include <sys/socket.h> // socket, bind, listen, inet_ntoa
#include <sys/time.h>
#include <sys/types.h> // socket, bind
#include <sys/uio.h>   // writev
#include <unistd.h>    // read, write, close
#include <cstring>
#include <ostream>
#include "TcpSocket.h"

using namespace std;

const char *PORT = "34371";
const char *CLIENT = "10.65.65.87"; // my IP address given by huky on net

const int TOTAL_MSG = 2000;
const int MSG_SIZE = 4;

int main()
{

   struct addrinfo hints, *res, *p;
   memset(&hints, 0, sizeof(hints)); // Zero-initialize hints

   hints.ai_family = AF_UNSPEC;     // use IPv4 or IPv6, whichever
   hints.ai_socktype = SOCK_STREAM; // use TCP
   hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

   // socket descriptor
   int sd;

   // call getaddrinfo to update res
   if (getaddrinfo(NULL, PORT, &hints, &res) < 0)
   {
      cerr << "Cannot resolve server info." << endl;
      return -1;
   }

   if ((sd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
   {
      cerr << "server: socket" << endl;
   }

   // lose the pesky "Address already in use" error message
   const int yes = 1;
   if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0)
   {
      cerr << "Set Socket Option Error!" << endl;
   }

   if (bind(sd, p->ai_addr, p->ai_addrlen) < 0)
   {
      close(sd);
      sd = NULL_SD;
      cerr << "server: bind" << endl;
   }

   // listen to up to 2 requests
   if (listen(sd, 2) < 0)
   {
      cerr << "listen error" << endl;
   }

   freeaddrinfo(res);
   if (sd < 0)
   {
      cerr << "server: failed to bind" << endl;
      return -1;
   }

   struct sockaddr_storage clientAddr;
   socklen_t clientAddrSize = sizeof(clientAddr);

   while (1)
   {
      int newSd = accept(sd, (struct sockaddr *)&clientAddr, &clientAddrSize);
      if (newSd < 0)
      {
         cerr << "socket connection error" << errno << endl;
         return -1;
      }

      // just be a server for this one socket forever
      while (1)
      {
         int msg[1];
         // keep reading buffer until we get our message
         for (int bytesRead = 0; bytesRead < MSG_SIZE; bytesRead +=
                                                       read(sd, (char *)msg, MSG_SIZE))
            ;
         // send the message back
         write(sd, (char *)msg, MSG_SIZE);
      }
   }
}
