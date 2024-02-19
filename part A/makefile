.PHONY: all clean

HEADERS = args_parser.h

all: TCP_Reciever TCP_Sender

TCP_Reciever: TCP_Reciever.o args_parser.o
	gcc -Wall -o TCP_Reciever TCP_Reciever.o args_parser.o

TCP_Sender: TCP_Sender.o args_parser.o
	gcc -Wall -o TCP_Sender TCP_Sender.o args_parser.o

%.o: %.c $(HEADERS)
	gcc -Wall -c $< -o $@

clean: 
	rm -f TCP_Reciever TCP_Sender *.o
