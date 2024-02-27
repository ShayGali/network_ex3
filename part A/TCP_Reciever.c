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

#include "args_parser.h"
const char EXIT_MESSAGE[] = {255, 0};

int connect_to_sndr(int port);

int main(int argc, char *argv[]) {
  clock_t start, end;
  double cpu_time_used;
  int port;
  int keep = 1;

  char *algo;
  if (!parse_args(argc, argv, &port, &algo)) {
    return 1;
  }

  printf("Port: %d\n", port);
  printf("Algorithm: %s\n", algo);

  /**
   * create a TCP socket btween the receiver and the sender
   */
  int sock = connect_to_sndr(port);
  if (sock == -1) {
    return 1;
  }

  while (keep) {
    size_t total_bytes = 0;
    char buffer[2097152];

    start = clock();

    while (total_bytes < 2092152) {
      ssize_t bytes_received =
          recv(sock, buffer + total_bytes, sizeof(buffer) - total_bytes, 0);
      if (total_bytes == 0 && buffer[0] == EXIT_MESSAGE[0]) {
        keep = 0;
        printf("exit message received\n");
        break;
      }
      if (bytes_received == -1) {
        perror("recv");
        return 1;
      }
      total_bytes += bytes_received;
    }
    end = clock();
    printf("file received\n");
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
  }

  return 0;
}

/**
 * @brief Create a TCP socket between the receiver and the sender
 * @param port The port to connect to
 * @return The socket file descriptor, or -1 on error
 */
int connect_to_sndr(int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return -1;
  }

  if (listen(sock, 1) == -1) {
    perror("listen");
    return -1;
  }

  struct sockaddr_in sender_addr;
  socklen_t sendder_addr_len = sizeof(sender_addr);
  int sender_sock =
      accept(sock, (struct sockaddr *)&sender_addr, &sendder_addr_len);
  if (sender_sock == -1) {
    perror("accept");
    return -1;
  }

  return sender_sock;
}
