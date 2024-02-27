/*The Sender will send a randomly generated file*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "args_parser.h"

#define RECEIVER_IP "127.0.0.1"
#define FILE_SIZE 2097152  // 2MB
/**
 * Create a TCP socket between the sender and the receiver
 */
int connect_to_recv(int port);
char *util_generate_random_data(unsigned int size);

/**
 * Send the file to the receiver
 */
void send_file();

int main(int argc, char *argv[]) {
  int port;
  char *algo;
  if (!parse_args(argc, argv, &port, &algo)) {
    return 1;
  }

  int file_data = util_generate_random_data(FILE_SIZE);
  int sock = connect_to_recv(port);
  if (sock == -1) {
    return 1;
  }
  /*
  ~ template code ~
  file = read_file();
  socket = create_socket();
  do{
    send_file(socket, file);
    user_decision = ask_user();
  } while(user_decision == 1);

  send_exit_message();
  close_socket();
  */

  free(file_data);
  return 0;
}

/*
 * @brief A random data generator function based on srand() and rand().
 * @param size The size of the data to generate (up to 2^32 bytes).
 * @return A pointer to the buffer.
 */
char *util_generate_random_data(unsigned int size) {
  char *buffer = NULL;
  // Argument check.
  if (size == 0) return NULL;
  buffer = (char *)calloc(size, sizeof(char));
  // Error checking.
  if (buffer == NULL) return NULL;
  // Randomize the seed of the random number generator.
  srand(time(NULL));
  for (unsigned int i = 0; i < size; i++)
    *(buffer + i) = ((unsigned int)rand() % 256);
  return buffer;
}

/**
 * Create a TCP socket between the sender and the receiver.
 * The sender will connect to the receiver.
 * @return the socket file descriptor, -1 if an error occurred
 */

int connect_to_recv(int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket creation failed");
    return -1;
  }

  // connect to the receiver
  struct sockaddr_in receiver_addr;
  receiver_addr.sin_family = AF_INET;    // IPv4
  receiver_addr.sin_port = htons(port);  // Convert port to network byte order
  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, RECEIVER_IP, &(receiver_addr.sin_addr)) <= 0) {
    perror("invalid address or address not supported");
    return -1;
  }

  // connect to the receiver
  if (connect(sock, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) <=
      0) {
    perror("connection failed");
    return -1;
  }

  return sock;
}