#!/bin/bash

g++ -pthread client.cpp Timer.cpp -o client
g++ server.cpp -o server
