# Makefile for TCP project

all: tcp_server tcp_client

tcp_server: tcp_server.c
	gcc -o tcp_server tcp_server.c

tcp_client: tcp_client.c
	gcc -o tcp_client tcp_client.c

clean:
	rm -f *.o tcp_server tcp_client