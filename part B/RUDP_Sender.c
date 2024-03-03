#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "RUDP_API.h"

#define FILE_SIZE 2097152  // 2MB
#define EXIT_MESSAGE "exit"

int parse_args(int argc, char *argv[], char **ip, int *port);
int connect_to_recv(char *ip, int port);
char *util_generate_random_data(unsigned int size);

int main(int argc, char *argv[]) {
  // Parse the command line arguments
  char *ip;
  int port;
  if (!parse_args(argc, argv, &ip, &port)) {
    return 1;
  }

  // Generate the file data
  char *file_data = util_generate_random_data(FILE_SIZE);

  // Create a RUDP socket between the sender and the receiver
  int sock = connect_to_recv(ip, port);
  if (sock == -1) {
    return 1;
  }

  int choice;
  do {
    printf("Sending the file\n");
    if (RUDP_send(sock, file_data, FILE_SIZE) < 0) {
      printf("Error sending the file\n");
      RUDP_close(sock);
      return 1;
    }
    printf("Do you want to send the file again? (1/0): ");
    scanf("%d", &choice);
  } while (choice == 1);

  printf("Closing the connection\n");
  RUDP_close(sock);
  printf("Connection closed\n");
  free(file_data);
  return 0;
}

/**
 * Create a RUDP socket and connect to the receiver.
 * The sender will connect to the receiver.
 * @return the socket file descriptor, -1 if an error occurred
 */
int connect_to_recv(char *ip, int port) {
  int sock = RUDP_socket();
  if (sock == -1) {
    return -1;
  }
  if (RUDP_connect(sock, ip, port) <= 0) {
    return -1;
  }
  return sock;
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
    *(buffer + i) = ((unsigned int)rand() % 255) + 1;
  return buffer;
}

/**
 * @brief Parse the port number from a string
 * @return the port number if it is valid, -1 otherwise
 */
int parse_port(char *port) {
  char *endptr;
  long int port_num = strtol(port, &endptr, 10);
  if (*endptr != '\0' || port_num < 0 || port_num > 65535) {
    return -1;
  }
  return (int)port_num;
}

/**
 * @brief Check if the arguments passed to the program are valid.
 * @param argc the number of arguments
 * @param argv the arguments passed to the program
 * @param port a pointer to the port number. It will be updated if the port is
 * valid
 * @param algo a pointer to the algorithm. It will be updated if the algorithm
 * is valid. a valid algorithm is either "reno" or "cubic"
 * @return 1 if the arguments are valid, 0 otherwise
 */
int parse_args(int argc, char *argv[], char **ip, int *port) {
  if (argc != 5 ||  // check if the number of arguments is correct
                    // check if the user passed the correct arguments
      (strcmp(argv[1], "-ip") != 0 && strcmp(argv[3], "-p") != 0)) {
    printf("Invalid arguments.\nUsage: %s -ip <IP> -p <port> \n", argv[0]);
    return 0;
  }

  // parse the port number, and check if it is valid
  // and store it in the port variable
  *ip = argv[2];
  *port = parse_port(argv[4]);
  if (*port == -1) {
    printf("Invalid port number\n");
    return 0;
  }
  return 1;
}