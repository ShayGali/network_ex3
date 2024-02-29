
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "stdio.h"

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 5060
#define BUFFER_SIZE 1024
typedef struct data {
  int id;
  char message[40];
} Data;

int main() {
  // Create socket
  // AF_INET: IPv4, SOCK_DGRAM: UDP, IPPROTO_UDP: UDP
  int listen_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (listen_socket == -1) {  // check for error
    printf("Could not create socket : %d\n", errno);
    return -1;
  }

  // setup Server address structure
  struct sockaddr_in serverAddress;
  // zero out the structure
  memset((char *)&serverAddress, 0, sizeof(serverAddress));
  // use IPv4
  serverAddress.sin_family = AF_INET;
  // (5001 = 0x89 0x13) little endian => (0x13 0x89) network endian (big endian)
  serverAddress.sin_port = htons(SERVER_PORT);
  // inet_pton: convert IPv4 and IPv6 addresses from text to binary form
  int ret = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS,
                      &(serverAddress.sin_addr));
  if (ret <= 0) {
    printf("inet_pton() failed\n");
    return -1;
  }

  // Bind
  int bindResult = bind(listen_socket, (struct sockaddr *)&serverAddress,
                        sizeof(serverAddress));
  if (bindResult == -1) {
    printf("bind() failed with error code : %d\n", errno);
    // cleanup the socket;
    close(listen_socket);
    return -1;
  }
  printf("After bind(). Waiting for clients\n");

  // setup Client address structure
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen = sizeof(clientAddress);

  // keep listening for data
  while (1) {
    fflush(stdout);  // clear the buffer

    // zero client address
    memset((char *)&clientAddress, 0, sizeof(clientAddress));

    Data *buffer = (Data *)malloc(sizeof(Data));

    // clear the buffer by filling null, it might have previously received data
    memset(buffer, 0, sizeof(Data));

    // try to receive some data, this is a blocking call
    // recvfrom: receive a message from a socket.
    // get the sokcet, buffer, buffer size, flags, client address, client
    // address length
    int recv_len =
        recvfrom(listen_socket, buffer, sizeof(Data) - 1, 0,
                 (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (recv_len == -1) {
      printf("recvfrom() failed with error code : %d", errno);
      // cleanup the socket;
      close(listen_socket);
      break;
    }

    // for store client IP address
    char clientIPAddrReadable[32] = {'\0'};
    // convert IPv4 and IPv6 addresses from binary to text form
    // and store it in clientIPAddrReadable
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIPAddrReadable,
              sizeof(clientIPAddrReadable));

    // print details of the client/peer and the data received
    printf("Received packet from %s:%d\n", clientIPAddrReadable,
           ntohs(clientAddress.sin_port));

    printf("id is: %d, Data is: %s\n", buffer->id, buffer->message);

    // now reply to the Client
    char message[] = "Hello from Server\n";
    int messageLen = strlen(message) + 1;
    // sendto: send a message on a socket.
    int sendResult =
        sendto(listen_socket, message, messageLen, 0,
               (struct sockaddr *)&clientAddress, clientAddressLen);

    if (sendResult == -1) {
      printf("sendto() failed with error code : %d\n", errno);
      break;
    }
  }
  // cleanup the socket
  close(listen_socket);
  return 0;
}
