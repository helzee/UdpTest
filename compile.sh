#!/bin/bash

g++ -pthread client.cpp Timer.cpp UdpSocket.cpp -o client
g++ server.cpp UdpSocket.cpp -o server
