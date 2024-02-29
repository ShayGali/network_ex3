.PHONY: all clean


all: RUDP_Reciever RUDP_Sender

RUDP_Reciever: RUDP_Reciever.o 
	gcc -Wall -o RUDP_Reciever RUDP_Reciever.o  RUDP_API.o

RUDP_Sender: RUDP_Sender.o 
	gcc -Wall -o RUDP_Sender RUDP_Sender.o RUDP_API.o

%.o: %.c RUDP_API.h
	gcc -Wall -c $< -o $@

clean: 
	rm -f RUDP_Reciever RUDP_Sender *.o
