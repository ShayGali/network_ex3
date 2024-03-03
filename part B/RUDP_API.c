#include "RUDP_API.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define TIMEOUT 1 // Timeout in seconds

int checksum(RUDP *packet);
int wait_for_ack(int socket, int seq_num, clock_t start_time, int timeout);
int send_ack(int socket, RUDP *packet);
int set_timeout(int socket, int time);

int sq_num = 0;

void print_RUDP(RUDP *packet) {
  printf("RUDP Packet:\n");
  printf("  Flags:\n");
  printf("    SYN: %u\n", packet->flags.SYN);
  printf("    ACK: %u\n", packet->flags.ACK);
  printf("    DATA: %u\n", packet->flags.DATA);
  printf("    FIN: %u\n", packet->flags.FIN);
  printf("  Sequence Number: %d\n", packet->seq_num);
  printf("  Checksum: %d\n", packet->checksum);
  printf("  Length: %d\n", packet->length);
  // printf("  Data: ");
  // for (int i = 0; i < packet->length; i++) {
  //   printf("%c", packet->data[i]);
  // }
  printf("\n");
}

int RUDP_socket() {
  // create udp socket
  int send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (send_socket == -1) {
    printf("Could not create socket : %d", errno);
    return ERROR;
  }

  return send_socket;
}

int RUDP_connect(int socket, char *ip, int port) {
  // setup a timeout for the socket
  if (set_timeout(socket, TIMEOUT) == ERROR) {
    return ERROR;
  }
  // Setup the server address structure.
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  int rval = inet_pton(AF_INET, (const char *)ip, &serverAddress.sin_addr);
  if (rval <= 0) {
    printf("inet_pton() failed");
    return ERROR;
  }

  if (connect(socket, (struct sockaddr *)&serverAddress,
              sizeof(serverAddress)) == -1) {
    perror("connect failed");
    return ERROR;
  }

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
      return -1;
    }
    clock_t start_time = clock();
    do {
      // receive SYN-ACK message
      recv_packet = malloc(sizeof(RUDP));
      memset(recv_packet, 0, sizeof(RUDP));
      int recv_result = recvfrom(socket, recv_packet, sizeof(RUDP), 0, NULL, 0);
      if (recv_result == -1) {
        perror("recvfrom failed");
        free(packet);
        free(recv_packet);
        return ERROR;
      }
      if (recv_packet->flags.SYN && recv_packet->flags.ACK) {
        printf("connected\n");
        free(packet);
        free(recv_packet);
        return SUCCESS;
      } else {
        printf("received wrong packet when trying to connect");
      }
    } while ((double)(clock() - start_time) / CLOCKS_PER_SEC < TIMEOUT);
    total_tries++;
  }

  printf("could not connect to receiver");
  free(packet);
  free(recv_packet);
  return FAILURE;
}

int RUDP_get_connection(int socket, int port) {
  // Setup the server address structure.
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind the socket to the server address
  int bindResult =
      bind(socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (bindResult == -1) {
    printf("bind() failed with error code : %d\n", errno);
    close(socket);
    return ERROR;
  }

  // receive SYN message
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen = sizeof(clientAddress);
  memset((char *)&clientAddress, 0, sizeof(clientAddress));

  RUDP *packet = malloc(sizeof(RUDP));
  memset(packet, 0, sizeof(RUDP));
  int recv_len = recvfrom(socket, packet, sizeof(RUDP) - 1, 0,
                          (struct sockaddr *)&clientAddress, &clientAddressLen);

  if (recv_len == -1) {
    printf("recvfrom() failed on receiving SYN with error code  : %d", errno);
    free(packet);
    return ERROR;
  }

  // make the connection for saving the client address
  if (connect(socket, (struct sockaddr *)&clientAddress, clientAddressLen) ==
      -1) {
    printf("connect() failed with error code  : %d", errno);
    free(packet);
    return ERROR;
  }

  // send SYN-ACK message
  if (packet->flags.SYN == 1) {
    RUDP *packetReply = malloc(sizeof(RUDP));
    memset(packet, 0, sizeof(RUDP));
    packetReply->flags.SYN = 1;
    packetReply->flags.ACK = 1;
    int sendResult = sendto(socket, packetReply, sizeof(RUDP), 0, NULL, 0);
    if (sendResult == -1) {
      printf("sendto() failed with error code  : %d", errno);
      free(packet);
      free(packetReply);
      return ERROR;
    }
    set_timeout(socket, TIMEOUT * 10);
    free(packet);
    free(packetReply);
    return SUCCESS;
  }

  return FAILURE;
}

// divide the data into packets and send them
int RUDP_send(int socket, char *data, int data_length) {
  int packets_num = data_length / WINDOW_MAX_SIZE;
  int last_packet_size = data_length % WINDOW_MAX_SIZE;
  RUDP *packet = malloc(sizeof(RUDP));
  for (int i = 0; i < packets_num; i++) {
    memset(packet, 0, sizeof(RUDP));
    packet->seq_num = i;
    packet->flags.DATA = 1;
    if (i == packets_num - 1 && last_packet_size == 0) {
      packet->flags.FIN = 1;
    }
    memcpy(packet->data, data + i * WINDOW_MAX_SIZE, WINDOW_MAX_SIZE);
    packet->length = WINDOW_MAX_SIZE;
    packet->length = WINDOW_MAX_SIZE;
    packet->checksum = checksum(packet);
    do {
      int sendResult = sendto(socket, packet, sizeof(RUDP), 0, NULL, 0);
      if (sendResult == -1) {
        printf("sendto() failed with error code  : %d", errno);
        return ERROR;
      }
      // wait for ack and retransmit if needed
    } while (wait_for_ack(socket, i, clock(), TIMEOUT) <= 0);
  }
  if (last_packet_size > 0) {
    memset(packet, 0, sizeof(RUDP));
    packet->seq_num = packets_num;
    packet->flags.DATA = 1;
    packet->flags.FIN = 1;
    memcpy(packet->data, data + packets_num * WINDOW_MAX_SIZE,
           last_packet_size);
    packet->length = last_packet_size;
    packet->checksum = checksum(packet);
    packet->length = last_packet_size;
    packet->checksum = checksum(packet);
    do {
      int sendResult = sendto(socket, packet, sizeof(RUDP), 0, NULL, 0);
      if (sendResult == -1) {
        printf("sendto() failed with error code  : %d", errno);
        free(packet);
        return ERROR;
      }
    } while (wait_for_ack(socket, packets_num, clock(), TIMEOUT) <= 0);
    free(packet);
  }
  return SUCCESS;
}

int RUDP_receive(int socket, char **data, int *data_length) {
  RUDP *packet = malloc(sizeof(RUDP));
  memset(packet, 0, sizeof(RUDP));

  int recvLen = recvfrom(socket, packet, sizeof(RUDP) - 1, 0, NULL, 0);
  if (recvLen == -1) {
    printf("recvfrom() failed on receiving data with error code  : %d", errno);
    free(packet);
    return -1;
  }
  // check if the packet is corrupted, and send ack
  if (checksum(packet) != packet->checksum) {
    free(packet);
    return -1;
  }
  if (send_ack(socket, packet) == -1) {
    free(packet);
    return -1;
  }
  if (packet->flags.SYN == 1) {  // connection request
    printf("received connection request\n");
    free(packet);
    return 0;
  }
  if (packet->seq_num == sq_num) {
    if (packet->seq_num == 0 && packet->flags.DATA == 1) {
      set_timeout(socket, TIMEOUT * 10);
    }
    if (packet->flags.FIN == 1 && packet->flags.DATA == 1) {  // last packet
      *data = malloc(packet->length);  // Allocate memory for data
      memcpy(*data, packet->data, packet->length);
      *data_length = packet->length;
      free(packet);
      sq_num = 0;
      set_timeout(socket, 10000000);
      return 2;
    }
    if (packet->flags.DATA == 1) {     // data packet
      *data = malloc(packet->length);  // Allocate memory for data
      memcpy(*data, packet->data, packet->length);
      *data_length = packet->length;
      free(packet);
      sq_num++;
      return 1;
    }
  } else if (packet->flags.DATA == 1) {
    free(packet);
    return 0;
  }
  if (packet->flags.FIN == 1) {  // close request
    free(packet);
    // send ack and wait for TIMEOUT*10 seconds to check if the sender closed
    printf("received close request\n");
    set_timeout(socket, TIMEOUT * 10);

    packet = malloc(sizeof(RUDP));
    time_t FIN_send_time = time(NULL);
    printf("waiting for close\n");
    while ((double)(time(NULL) - FIN_send_time) < TIMEOUT) {
      memset(packet, 0, sizeof(RUDP));
      recvfrom(socket, packet, sizeof(RUDP) - 1, 0, NULL, 0);
      if (packet->flags.FIN == 1) {
        if (send_ack(socket, packet) == -1) {
          free(packet);
          return -1;
        }
        FIN_send_time = time(NULL);
      }
    }
    free(packet);
    close(socket);
    return -2;
  }
  free(packet);
  return 0;
}

// close the connection
int RUDP_close(int socket) {
  RUDP *close_packet = malloc(sizeof(RUDP));
  memset(close_packet, 0, sizeof(RUDP));
  close_packet->flags.FIN = 1;
  close_packet->seq_num = -1;
  close_packet->checksum = checksum(close_packet);
  do {
    int sendResult = sendto(socket, close_packet, sizeof(RUDP), 0, NULL, 0);
    if (sendResult == -1) {
      printf("sendto() failed with error code  : %d", errno);
      free(close_packet);
      return -1;
    }
  } while (wait_for_ack(socket, -1, clock(), TIMEOUT) <= 0);
  close(socket);
  free(close_packet);
  return SUCCESS;
}

// calculate the checksum of the packet in the most simple way !!!
int checksum(RUDP *packet) {
  int sum = 0;
  for (int i = 0; i < 10 && i < WINDOW_MAX_SIZE; i++) {
    sum += packet->data[i];
  }
  return sum;
}

int wait_for_ack(int socket, int seq_num, clock_t start_time, int timeout) {
  RUDP *packetReply = malloc(sizeof(RUDP));
  while ((double)(clock() - start_time) / CLOCKS_PER_SEC < timeout) {
    int recvLen = recvfrom(socket, packetReply, sizeof(RUDP) - 1, 0, NULL, 0);
    if (recvLen == -1) {
      free(packetReply);
      return FAILURE;
    }
    if (packetReply->seq_num == seq_num && packetReply->flags.ACK == 1) {
      free(packetReply);
      return SUCCESS;
    }
  }
  free(packetReply);
  return FAILURE;
}

int send_ack(int socket, RUDP *packet) {
  RUDP *ack_packet = malloc(sizeof(RUDP));
  memset(ack_packet, 0, sizeof(RUDP));
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
  int sendResult = sendto(socket, ack_packet, sizeof(RUDP), 0, NULL, 0);
  if (sendResult == -1) {
    printf("sendto() failed with error code  : %d", errno);
    free(ack_packet);
    return FAILURE;
  }
  free(ack_packet);
  return SUCCESS;
}

int set_timeout(int socket, int time) {
  // set timeout for the socket
  struct timeval timeout;
  timeout.tv_sec = time;
  timeout.tv_usec = 0;

  if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
      0) {
    perror("Error setting timeout for socket");
    return ERROR;
  }
  return SUCCESS;
}