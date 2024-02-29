
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
  int send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (send_socket == -1) {
    printf("Could not create socket : %d", errno);
    return -1;
  }

  // Setup the server address structure.
  // Port and IP should be filled in network byte order (learn bin-endian,
  // little-endian)
  //
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(SERVER_PORT);
  int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS,
                       &serverAddress.sin_addr);
  if (rval <= 0) {
    printf("inet_pton() failed");
    return -1;
  }

  // send the message
  char message[] = "GGGGG";
  Data *data = (Data *)malloc(sizeof(Data));
  data->id = 2;
  strcpy(data->message, message);
  int messageLen = sizeof(Data);

  int sendResult =
      sendto(send_socket, data, messageLen, 0,
             (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  free(data);

  if (sendResult == -1) {
    printf("sendto() failed with error code  : %d", errno);
    return -1;
  }

  struct sockaddr_in fromAddress;
  // Change type variable from int to socklen_t: int fromAddressSize =
  // sizeof(fromAddress);
  socklen_t fromAddressSize = sizeof(fromAddress);

  memset((char *)&fromAddress, 0, sizeof(fromAddress));

  // try to receive some data, this is a blocking call
  char bufferReply[BUFFER_SIZE] = {'\0'};
  int recvLen = recvfrom(send_socket, bufferReply, sizeof(bufferReply) - 1, 0,
                         (struct sockaddr *)&fromAddress, &fromAddressSize);
  if (recvLen == -1) {
    printf("recvfrom() failed with error code  : %d", errno);
    return -1;
  }
  // print the ip of the sender the port and the data received
  printf("[%s:%d] Data: %s", inet_ntoa(fromAddress.sin_addr),
         ntohs(fromAddress.sin_port), bufferReply);

  close(send_socket);

  return 0;
}
