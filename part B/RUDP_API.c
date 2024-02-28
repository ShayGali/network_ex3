#include "RUDP_API.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define WINDOW_MAX_SIZE 1024  // CHECK

int RUDP_socket(char *ip, int port) {
  int send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (send_socket == -1) {
    printf("Could not create socket : %d", errno);
    return -1;
  }
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  int rval = inet_pton(AF_INET, (const char *)ip, &serverAddress.sin_addr);
  if (rval <= 0) {
    printf("inet_pton() failed");
    return -1;
  }
  return send_socket;
}

int checksum(RUDP *packet) {
  int sum = 0;
  for (int i = 0; i < 10 && i < sizeof(packet->data); i++) {
    sum += packet->data[i];
  }
  return sum;
}