#! /bin/bash

make all
echo "Running TCP_Reciever"
./TCP_Reciever -p 5050 -algo reno
echo "Running TCP_Sender"
./TCP_Sender -ip 127.0.0.1 -p 5050 -algo reno

make clean
