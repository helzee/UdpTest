#include "Timer.h"
#include "UdpSocket.h"
#include <pthread.h>
#include <iostream>
#include <fstream>

using namespace std;

const char *PORT = "34369";
const char *RECV_PORT = "34370";
const char *SERVER = "10.65.68.231";
const int TOTAL_MSG = 10000;
const int MSG_SIZE = 4;
const int INTERVAL = 100000;

bool keepReceiving = true;

struct thread_data
{
   Timer *allTimers;
   long *allRTT;
};

void *receiverThreadFunc(void *ptr)
{

   thread_data *data = (thread_data *)ptr;

   UdpSocket udpSock(RECV_PORT);

   int msg[1];

   cout << "thread start" << endl;
   while (keepReceiving)
   {
      while (udpSock.pollRecvFrom())
      {
         udpSock.recvFrom((char *)msg, MSG_SIZE);
         int index = msg[0];
         data->allRTT[index] = data->allTimers[index].lap();
         cout << msg[0] << endl;
      }
   }

   return ptr;
}

int main()
{
   UdpSocket udpSock(PORT);
   udpSock.setDestAddress(SERVER);

   // test sending a byte of data to server
   // measure latency and dropped messages

   int ack[1];
   int msg[1];

   Timer wait;

   // shared data for both threads
   Timer *allTimers = new Timer[TOTAL_MSG];
   long *allRTT = new long[TOTAL_MSG];
   for (int i = 0; i < TOTAL_MSG; i++)
   {
      allRTT[i] = 0;
   }

   pthread_t recvThread;
   struct thread_data *data = new thread_data;
   data->allTimers = allTimers;
   data->allRTT = allRTT;

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
      udpSock.sendTo((char *)msg, MSG_SIZE);
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