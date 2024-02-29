#include "RUDP_API.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

int RUDP_connect(int socket) {
  // send SYN message
  RUDP *packet = malloc(sizeof(RUDP));
  memset(packet, 0, sizeof(RUDP));
  packet->flags.SYN = 1;

  int total_tries = 0;
  RUDP *recv_packet = NULL;
  while (total_tries < RETRY) {
    int send_result = sendto(socket, packet, sizeof(RUDP), 0, NULL, 0);
    if (send_result == -1) {
      perror("sendto failed");
      free(packet);
      return 0;
    }

    // receive SYN-ACK message
    recv_packet = malloc(sizeof(RUDP));
    memset(recv_packet, 0, sizeof(RUDP));
    int recv_result = recvfrom(socket, recv_packet, sizeof(RUDP), 0, NULL, 0);
    if (recv_result == -1) {
      perror("recvfrom failed");
      free(recv_packet);
      free(packet);
      return 0;
    }
    if (recv_packet->flags.SYN && recv_packet->flags.ACK) {
      free(recv_packet);
      free(packet);
      return 1;
    } else {
      printf("received wrong packet when trying to connect");
    }
    free(recv_packet);
    total_tries++;
  }

  free(packet);
  return 0;
}
int checksum(RUDP *packet) {
  int sum = 0;
  for (int i = 0; i < 10 && i < sizeof(packet->data); i++) {
    sum += packet->data[i];
  }
  return sum;
}