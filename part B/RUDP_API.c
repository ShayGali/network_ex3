#include "RUDP_API.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define WINDOW_MAX_SIZE 1024  // CHECK
#define TIMEOUT 1000

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
  RUDP *packet;
  memset(packet, 0, sizeof(packet));
  packet->flags.SYN = 1;

  int total_tries = 0;
  while (total_tries < RETRY) {
    int send_result = sendto(socket, packet, sizeof(packet), 0, NULL, 0);
    if (send_result == -1) {
      perror("sendto failed");
      return 0;
    }

    // receive SYN-ACK message
    RUDP *recv_packet;
    memset(recv_packet, 0, sizeof(recv_packet));
    int recv_result =
        recvfrom(socket, recv_packet, sizeof(recv_packet), 0, NULL, 0);
    if (recv_result == -1) {
      perror("recvfrom failed");
      return 0;
    }
    if (recv_packet->flags.SYN && recv_packet->flags.ACK) {
      return 1;
    } else {
      printf("received wrong packet when trying to connect");
    }
    total_tries++;
  }

  // failed to connect
  return 0;
}

int RUDP_send(int socket, char *data, int data_length) {
  int packets_num = data_length / WINDOW_MAX_SIZE;
  int last_packet_size = data_length % WINDOW_MAX_SIZE;
  for (int i = 0; i < packets_num; i++) {
    RUDP packet;
    memset(&packet, 0, sizeof(packet));
    packet.seq_num = i;
    packet.flags.DATA = 1;
    packet.checksum = checksum(&packet);
    memcpy(packet.data, data + i * WINDOW_MAX_SIZE, WINDOW_MAX_SIZE);
    do {
      int sendResult = sendto(socket, &packet, sizeof(packet), 0, NULL, 0);
      if (sendResult == -1) {
        printf("sendto() failed with error code  : %d", errno);
        return -1;
      }
    } while (wait_for_ack(socket, i, clock(), TIMEOUT) == -1);
  }
  if (last_packet_size > 0) {
    RUDP packet;
    packet.seq_num = packets_num;
    packet.checksum = checksum(&packet);
    memcpy(packet.data, data + packets_num * WINDOW_MAX_SIZE, last_packet_size);
    do {
      int sendResult = sendto(socket, &packet, sizeof(packet), 0, NULL, 0);
      if (sendResult == -1) {
        printf("sendto() failed with error code  : %d", errno);
        return -1;
      }
    } while (wait_for_ack(socket, packets_num, clock(), TIMEOUT) == -1);
  }
  return 0;
}

int checksum(RUDP *packet) {
  int sum = 0;
  for (int i = 0; i < 10 && i < sizeof(packet->data); i++) {
    sum += packet->data[i];
  }
  return sum;
}

int wait_for_ack(int socket, int seq_num, clock_t start_time, int timeout) {
  RUDP *packetReply;
  while (clock() - start_time < timeout) {
    int recvLen =
        recvfrom(socket, packetReply, sizeof(packetReply) - 1, 0, NULL, 0);
    if (recvLen == -1) {
      printf("recvfrom() failed with error code  : %d", errno);
      return -1;
    }
    if (packetReply->seq_num == seq_num && packetReply->flags.ACK == 1) {
      return 0;
    }
  }
  return -1;
}