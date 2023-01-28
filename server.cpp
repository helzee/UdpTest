#include "Timer.h"
#include "UdpSocket.h"
#include <ostream>

using namespace std;

const char* PORT = "34369";
const char* RECV_PORT = "34370";
const char* CLIENT = "10.65.65.87"; // my IP address given by huky on net

const int MSG_SIZE = 2000;

int main()
{
   UdpSocket udpSock(PORT);
   UdpSocket reciever(RECV_PORT);
   reciever.setDestAddress(CLIENT);

   int msg[1];

   // udpSock.recvFrom((char*)msg, 4);
   // reciever.setSrcAddr(udpSock.getSrcAddr());
   // if (reciever.sendTo((char*)msg, 4) < 0) {
   //    cerr << "error sending msg";
   // }
   // cout << msg[0] << endl;

   while (true) {
      while (udpSock.pollRecvFrom()) {
         udpSock.recvFrom((char*)msg, 4);
         if (reciever.sendTo((char*)msg, 4) < 0) {
            cerr << "error sending msg" << endl;
         }
         cout << msg[0] << endl;
      }
   }
}