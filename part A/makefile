.PHONY: all clean


all: TCP_Reciever TCP_Sender

TCP_Reciever: TCP_Reciever.o 
	gcc -Wall -o TCP_Reciever TCP_Reciever.o 

TCP_Sender: TCP_Sender.o 
	gcc -Wall -o TCP_Sender TCP_Sender.o 

%.o: %.c
	gcc -Wall -c $< -o $@

clean: 
	rm -f TCP_Reciever TCP_Sender *.o
