
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

int main() {
  int s = -1;

  // Create socket
  s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == -1) {
    printf("Could not create socket : %d\n", errno);
    return -1;
  }

  // setup Server address structure
  struct sockaddr_in serverAddress;
  memset((char *)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port =
      htons(SERVER_PORT);  // (5001 = 0x89 0x13) little endian => (0x13 0x89)
                           // network endian (big endian)
  int ret = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS,
                      &(serverAddress.sin_addr));
  if (ret <= 0) {
    printf("inet_pton() failed\n");
    return -1;
  }

  // Bind
  int bindResult =
      bind(s, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (bindResult == -1) {
    printf("bind() failed with error code : %d\n", errno);
    // cleanup the socket;
    close(s);
    return -1;
  }
  printf("After bind(). Waiting for clients\n");

  // setup Client address structure
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen = sizeof(clientAddress);

  memset((char *)&clientAddress, 0, sizeof(clientAddress));

  // keep listening for data
  while (1) {
    fflush(stdout);

    // zero client address
    memset((char *)&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);

    char buffer[BUFFER_SIZE] = {'\0'};

    // clear the buffer by filling null, it might have previously received data
    memset(buffer, '\0', sizeof(buffer));

    int recv_len = -1;

    // try to receive some data, this is a blocking call
    recv_len = recvfrom(s, buffer, sizeof(buffer) - 1, 0,
                        (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (recv_len == -1) {
      printf("recvfrom() failed with error code : %d", errno);
      // cleanup the socket;
      close(s);
      break;
    }

    char clientIPAddrReadable[32] = {'\0'};
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIPAddrReadable,
              sizeof(clientIPAddrReadable));

    // print details of the client/peer and the data received
    printf("Received packet from %s:%d\n", clientIPAddrReadable,
           ntohs(clientAddress.sin_port));
    printf("Data is: %s\n", buffer);

    // now reply to the Client
    char message[] = "Hello from Server\n";
    int messageLen = strlen(message) + 1;
    int sendResult =
        sendto(s, message, messageLen, 0, (struct sockaddr *)&clientAddress,
               clientAddressLen);
    if (sendResult == -1) {
      printf("sendto() failed with error code : %d\n", errno);
      break;
    }
  }
  close(s);
  return 0;
}
