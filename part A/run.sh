#! /bin/bash
PORT="5060"

make all
echo "Running TCP_Reciever"
./TCP_Reciever -p $PORT -algo reno
echo "Running TCP_Sender"
./TCP_Sender -p $PORT -algo cubic

make clean
