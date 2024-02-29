#include "RUDP_API.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
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
  if (connect(send_socket, (struct sockaddr *)&serverAddress,
              sizeof(serverAddress)) < 0) {
    perror("UDP connect failed");
    return -1;
  }

  struct timeval timeout;
  timeout.tv_sec = TIMEOUT;  // Timeout in seconds
  timeout.tv_usec = 0;

  if (setsockopt(send_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    perror("Error setting timeout for socket");
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
      return -1;
    }
    clock_t start_time = clock();
    do {
      // receive SYN-ACK message
      RUDP *recv_packet;
      memset(recv_packet, 0, sizeof(recv_packet));
      int recv_result =
          recvfrom(socket, recv_packet, sizeof(recv_packet), 0, NULL, 0);
      if (recv_result == -1) {
        perror("recvfrom failed");
        return -1;
      }
      if (recv_packet->flags.SYN && recv_packet->flags.ACK) {
        printf("connected");
        return 0;
      } else {
        printf("received wrong packet when trying to connect");
      }
    } while (clock() - start_time < TIMEOUT);
    total_tries++;
  }

  printf("could not connect to receiver");
  return -1;
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

int RUDP_receive(int socket, char *data, int data_length) {
  RUDP *packet;
  memset(packet, 0, sizeof(packet));
  int recvLen = recvfrom(socket, packet, sizeof(packet) - 1, 0, NULL, 0);
  if (recvLen == -1) {
    printf("recvfrom() failed with error code  : %d", errno);
    return -1;
  }
  if (checksum(packet) != packet->checksum) {
    return -1;
  }
  if (send_ack(socket, packet) == -1) {
    return -1;
  }
  if (packet->flags.SYN == 1) {  // connection request
    printf("received connection request");
    return 0;
  }
  if (packet->flags.DATA == 1) {  // data packet
    memcpy(data, packet->data, sizeof(packet->data));
    return 0;
  }
  if (packet->flags.FIN == 1) {  // close request
    clock_t FIN_send_time = clock();
    while (clock() - FIN_send_time < TIMEOUT * 10) {
      RUDP *packet;
      memset(packet, 0, sizeof(packet));
      int recvLen = recvfrom(socket, packet, sizeof(packet) - 1, 0, NULL, 0);
      if (recvLen == -1) {
        printf("recvfrom() failed with error code  : %d", errno);
        return -1;
      }
      if (packet->flags.FIN == 1) {
        if (send_ack(socket, packet) == -1) {
          return -1;
        }
        FIN_send_time = clock();
      }
    }
    return -1;
  }
  return 0;
}

int RUDP_close(int socket) {
  RUDP *close_packet;
  memset(close_packet, 0, sizeof(close_packet));
  close_packet->flags.FIN = 1;
  close_packet->seq_num = -1;
  close_packet->checksum = checksum(close_packet);
  do {
    int sendResult =
        sendto(socket, close_packet, sizeof(close_packet), 0, NULL, 0);
    if (sendResult == -1) {
      printf("sendto() failed with error code  : %d", errno);
      return -1;
    }
  } while (wait_for_ack(socket, -1, clock(), TIMEOUT) == -1);
  close(socket);
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

int send_ack(int socket, RUDP *packet) {
  RUDP *ack_packet;
  memset(ack_packet, 0, sizeof(ack_packet));
  ack_packet->flags.ACK = 1;
  if (packet->flags.FIN == 1) {
    ack_packet->flags.FIN = 1;
  }
  if (packet->flags.SYN == 1) {
    ack_packet->flags.SYN = 1;
  }
  if (packet->flags.DATA == 1) {
    ack_packet->flags.DATA = 1;
  }
  ack_packet->seq_num = packet->seq_num;
  ack_packet->checksum = checksum(ack_packet);
  int sendResult = sendto(socket, ack_packet, sizeof(ack_packet), 0, NULL, 0);
  if (sendResult == -1) {
    printf("sendto() failed with error code  : %d", errno);
    return -1;
  }
  return 0;
}