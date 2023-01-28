#include "Timer.h"
#include "TcpSocket.h"
#include <pthread.h>
#include <fstream>

using namespace std;

const char *PORT = "34371";

const char *SERVER = "10.65.68.231";
const int TOTAL_MSG = 20000;
const int MSG_SIZE = 4;
const int INTERVAL = 100000;

bool keepReceiving = true;

struct thread_data
{
   Timer *allTimers;
   long *allRTT;
   int sd;
};

void *receiverThreadFunc(void *ptr)
{

   thread_data *data = (thread_data *)ptr;

   int sd = data->sd;

   int msg[1];
   msg[0] = -1;

   cout << "thread start" << endl;
   while (keepReceiving)
   {

      for (int nRead = 0; nRead < MSG_SIZE; nRead += recv(sd, (char *)msg, MSG_SIZE, 0))
      {
      }

      int index = msg[0];
      data->allRTT[index] = data->allTimers[index].lap();
      cout << msg[0] << endl;
   }

   return ptr;
}

int createNewSocket(addrinfo *servInfo)
{
   // make a socket, bind it, listen to it
   int clientSd = socket(servInfo->ai_family, servInfo->ai_socktype,
                         servInfo->ai_protocol);
   if (clientSd < 0)
   {
      cerr << "Socket creation error!" << errno << endl;

      return -1;
   }
   // lose pesky "Address already in use" error message
   int status = connect(clientSd, servInfo->ai_addr, servInfo->ai_addrlen);
   if (status < 0)
   {
      cerr << "Failed to connect to the server" << errno << endl;

      return -1;
   }

   return clientSd;
}

int createClientTcpSocket(const char *port, const char *server)
{
   // create server info structure
   struct addrinfo hints, *servInfo; // loaded with getaddrinfo()
   memset(&hints, 0, sizeof(hints)); // initialize memory in struct hints
   hints.ai_family = AF_UNSPEC;      // use IPv4 or IPv6
   hints.ai_socktype = SOCK_STREAM;  // use TCP
   hints.ai_flags = AI_PASSIVE;

   // call getaddrinfo() to update servInfo
   int error = getaddrinfo(server, port, &hints, &servInfo);
   if (error != 0)
   {
      cerr << "getaddrinfo() Error! " << error << endl;
      exit(1);
   }

   return createNewSocket(servInfo);
}

int main()
{
   int sd = createClientTcpSocket(PORT, SERVER);
   if (sd < 0)
   {
      cerr << "socket create error " << errno << endl;
      return -1;
   }

   // test sending a byte of data to server
   // measure latency and dropped messages

   int msg[1];

   Timer wait;

   // shared data for both threads
   Timer *allTimers = new Timer[TOTAL_MSG]();
   long *allRTT = new long[TOTAL_MSG];
   for (int i = 0; i < TOTAL_MSG; i++)
   {
      allRTT[i] = 0;
      }

   pthread_t recvThread;
   struct thread_data *data = new thread_data;
   data->allTimers = allTimers;
   data->allRTT = allRTT;
   data->sd = sd;

   // create reciever thread
   int iret1 =
       pthread_create(&recvThread, NULL, receiverThreadFunc, (void *)data);

   /**
    * @brief We send out a bunch of messages in 0.5 second intervals.
    * At the same time, we are receiving messages and recording the time it
    * took to receive each message After the last message is sent, we have
    * half a second to recieve more messages before finishing. Any messages
    * recieved after that are useless
    *
    */

   for (int i = 0; i < TOTAL_MSG; i++)
   {
      msg[0] = i;
      // start timer for this message
      allTimers[i].start();
      // send message
      if (send(sd, (char *)msg, MSG_SIZE, 0) == -1)
      {
         cerr << "SEND ERROR " << errno << endl;
         return -1;
      }
      // wait 100 ms
      usleep(INTERVAL);
   }
   // stop receiver thread
   keepReceiving = false;

   long avgLatency;
   double sum = 0;
   int errors = 0;
   ofstream allOutput("all_output.csv");
   allOutput << "message number,latency (usec),message size,interval (usec),\n";
   for (int i = 0; i < TOTAL_MSG; i++)
   {
      allOutput << i << ",";
      if (allRTT[i] <= 0)
      {
         allOutput << "NULL,";
         cerr << "RRT negative or zero at i = " << i << ". RTT = " << allRTT[i]
              << endl;
         errors++;
      }
      else
      {
         sum += (double)allRTT[i] / 2;
         allOutput << (double)allRTT[i] / 2 << ",";
      }
      allOutput << MSG_SIZE << "," << INTERVAL << ",\n";
   }

   allOutput.close();

   cout << "Average latency (usec): " << (sum / TOTAL_MSG) << endl;
   cout << "Dropped messages: " << errors << endl;

   ofstream out("output.csv");
   out << "Average latency (usec),dropped messages, total messages, message size,interval (usec)\n";
   out << (sum / TOTAL_MSG) << ", " << errors << ", " << TOTAL_MSG << ", " << MSG_SIZE << "," << INTERVAL << ",\n ";
   out.close();

   delete[] allTimers;
   delete[] allRTT;

   return 0;
}