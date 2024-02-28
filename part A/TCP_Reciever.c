/* Revice a randomly generated file and
measure the time it took for his program to receive the file.

The receiver doesn’t really care about saving the file itself (or its content)

to run the program:
./TCP_Receiver -p <port> -algo <algorithm>
where:
<port> is the port number to listen on - a number between 0 and 65535
<algorithm> is the congestion control algorithm to use - either reno or cubic
*/
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

// the exit message that the sender will send
#define EXIT_MESSAGE "exit"

int connect_to_sender(int port);
int parse_args(int argc, char *argv[], int *port, char **algo);

int main(int argc, char *argv[]) {
  printf("~~~~~~~~ TCP Receiver ~~~~~~~~\n");

  clock_t start, end;  // to measure the time it took to receive the file
  double cpu_time_used;

  // Parse the command line arguments
  int port;
  char *algo;
  if (!parse_args(argc, argv, &port, &algo)) {
    return 1;
  }

  // create a TCP socket between the receiver and the sender
  int sock = connect_to_sender(port);
  if (sock == -1) {
    return 1;
  }

  // main loop to receive the file
  int run = 1;
  int listen = 1;
  char buffer[2097152];  // 2MB buffer to receive the file

  FILE *file = fopen("stats", "w+");
  if (file == NULL) {
    printf("Error opening  stats file!\n");
    return 1;
  }
  fprintf(file, "\n\n~~~~~~~~ Statistics ~~~~~~~~\n");
  double average_time = 0;
  double average_speed = 0;

  while (listen) {
    size_t total_bytes = 0;  // the total bytes received so far

    start = clock();  // start measuring the time

    // check if the exit message was received
    ssize_t bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    total_bytes += bytes_received;
    if (bytes_received == -1) {  // check for errors
      perror("recv");
      return 1;
    }

    if (strncmp(buffer, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0) {
      listen = 0;
      printf("exit message received\n");
      break;
    }
    // keep receiving until the file is received
    while (total_bytes < sizeof(buffer)) {
      bytes_received =
          recv(sock, buffer + total_bytes, sizeof(buffer) - total_bytes, 0);

      if (bytes_received == -1) {  // check for errors
        perror("recv");
        return 1;
      }
      if (bytes_received == 0) {
        printf("sender closed the connection\n");
        // check if the sender closed the connection
        break;
      }

      total_bytes += bytes_received;
    }
    // stop measuring the time
    end = clock();
    if (listen) {
      printf("file received\n");
      cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
      fprintf(file, "Run #%d Data: Time=%f S ; Speed=%f MB/S\n", run,
              cpu_time_used, (2 / cpu_time_used));
      average_time += cpu_time_used;
      average_speed += (2 / cpu_time_used);
      run++;
    }
  }
  // add the average time and speed to the file
  fprintf(file, "Average time: %f S\n", average_time / (run - 1));
  fprintf(file, "Average speed: %f MB/S\n", average_speed / (run - 1));

  // end the file with nice message
  fprintf(file, "\n\n-----------------------------\n");
  fprintf(file, "Congestion control algorithm: %s\n", algo);
  fprintf(file, "Thank you for using our service\n");

  // print the statistics
  rewind(file);
  char print_buffer[100];
  while (fgets(print_buffer, 100, file) != NULL) {
    printf("%s", print_buffer);
  }

  // Close the file
  fclose(file);
  remove("stats");
  // Close the socket
  close(sock);

  return 0;
}

/**
 * @brief Create a TCP socket between the receiver and the sender
 * will listen on the given port ( and local IP address) and wait for the sender
 * to connect
 * @param port The port to connect to
 * @return The socket of the connection to the sender, or -1 on error
 */
int connect_to_sender(int port) {
  // create a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return -1;
  }

  // create the address structure
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  // bind the socket to the address
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return -1;
  }

  if (listen(sock, 1) == -1) {
    perror("listen");
    return -1;
  }

  // wait for the sender to connect
  struct sockaddr_in sender_addr;
  socklen_t sendder_addr_len = sizeof(sender_addr);
  int sender_sock =
      accept(sock, (struct sockaddr *)&sender_addr, &sendder_addr_len);
  if (sender_sock == -1) {
    perror("accept");
    return -1;
  }
  close(sock);
  return sender_sock;
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
int parse_args(int argc, char *argv[], int *port, char **algo) {
  if (argc != 5 ||  // check if the number of arguments is correct
                    // check if the user passed the correct arguments
      (strcmp(argv[1], "-p") != 0 && strcmp(argv[1], "-algo") != 0) ||
      (strcmp(argv[3], "-p") != 0 && strcmp(argv[3], "-algo") != 0)) {
    printf("Invalid arguments.\nUsage: %s -p <port> -algo algorithm\n",
           argv[0]);
    return 0;
  }

  // parse the port number, and check if it is valid
  // and store it in the port variable
  if (strcmp(argv[1], "-p") == 0) {
    *port = parse_port(argv[2]);
    if (*port == -1) {
      printf("Invalid port number\n");
      return 0;
    }
    if (!is_valid_algo(argv[4])) {
      printf("Invalid algorithm\n");
      return 0;
    }
    *algo = argv[4];
  } else {
    *port = parse_port(argv[4]);
    if (*port == -1) {
      printf("Invalid port number\n");
      return 0;
    }
    if (!is_valid_algo(argv[2])) {
      printf("Invalid algorithm\n");
      return 0;
    }
    *algo = argv[2];
  }

  return 1;
}
