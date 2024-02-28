/*The Sender will send a randomly generated file*/

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define FILE_SIZE 2097152  // 2MB
#define EXIT_MESSAGE "exit"

int parse_args(int argc, char *argv[], char **ip, int *port, char **algo);
int connect_to_recv(char *ip, int port);
char *util_generate_random_data(unsigned int size);
void send_exit_message(int sock);
void send_file(int sock, char *file_data);

int main(int argc, char *argv[]) {
  // Parse the command line arguments
  char *ip;
  int port;
  char *algo;
  if (!parse_args(argc, argv, &ip, &port, &algo)) {
    return 1;
  }

  // Generate the file data
  char *file_data = util_generate_random_data(FILE_SIZE);

  // Create a TCP socket between the sender and the receiver
  int sock = connect_to_recv(ip, port);
  if (sock == -1) {
    return 1;
  }

  // define the congestion control algorithm used by a socket
  if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0) {
    perror("setsockopt");
    return 1;
  }

  int choice;
  do {
    send_file(sock, file_data);
    printf("Do you want to send the file again? (1/0): ");
    scanf("%d", &choice);
  } while (choice == 1);

  send_exit_message(sock);
  close(sock);
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
    *(buffer + i) = ((unsigned int)rand() % 255) + 1;
  return buffer;
}

/**
 * Create a TCP socket between the sender and the receiver.
 * The sender will connect to the receiver.
 * @return the socket file descriptor, -1 if an error occurred
 */
int connect_to_recv(char *ip, int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("socket creation failed\n");
    return -1;
  }

  // connect to the receiver
  struct sockaddr_in receiver_addr;
  receiver_addr.sin_family = AF_INET;    // IPv4
  receiver_addr.sin_port = htons(port);  // Convert port to network byte order
  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, ip, &(receiver_addr.sin_addr)) <= 0) {
    perror("invalid address or address not supported\n");
    return -1;
  }

  // connect to the receiver
  if (connect(sock, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) <
      0) {
    printf("connection failed\n");
    return -1;
  }

  return sock;
}

/**
 * Send the file to the receiver.
 * @param sock the socket file descriptor
 * @param file_data the file data to send
 */
void send_file(int sock, char *file_data) {
  size_t bytes_sent = 0;

  // loop until all the data is sent
  while (bytes_sent < FILE_SIZE) {
    // send the data
    ssize_t result =
        send(sock, file_data + bytes_sent, FILE_SIZE - bytes_sent, 0);
    if (result < 0) {  // if an error occurred
      break;
    }
    // update the number of bytes sent
    bytes_sent += result;
  }
}

/**
 * Send an exit message to the receiver.
 *
 * @param sock the socket file descriptor
 */
void send_exit_message(int sock) {
  send(sock, EXIT_MESSAGE, strlen(EXIT_MESSAGE), 0);
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
 * @brief Check if the algorithm is valid.
 * a valid algorithm is either "reno" or "cubic"
 * @return 1 if the algorithm is valid, 0 otherwise

*/
int is_valid_algo(char *algo) {
  if (strcmp(algo, "reno") != 0 && strcmp(algo, "cubic") != 0) {
    return 0;
  }
  return 1;
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
int parse_args(int argc, char *argv[], char **ip, int *port, char **algo) {
  if (argc != 7 ||  // check if the number of arguments is correct
                    // check if the user passed the correct arguments
      (strcmp(argv[1], "-ip") != 0 && strcmp(argv[3], "-p") != 0 &&
       strcmp(argv[5], "-algo") != 0)) {
    printf(
        "Invalid arguments.\nUsage: %s -ip <IP> -p <port> -algo <algorithm>\n",
        argv[0]);
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
  if (!is_valid_algo(argv[6])) {
    printf("Invalid algorithm\n");
    return 0;
  }
  *algo = argv[6];
  return 1;
}
