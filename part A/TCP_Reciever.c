/* Revice a randomly generated file and
measure the time it took for his program to receive the file.

The receiver doesn’t really care about saving the file itself (or its content)

to run the program:
./TCP_Receiver -p <port> -algo <algorithm>
where:
<port> is the port number to listen on - a number between 0 and 65535
<algorithm> is the congestion control algorithm to use - either reno or cubic
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_parser.h"

int check_args(int argc, char *argv[], int *port, char **algo);
int parse_port(char *port);
int is_valid_algo(char *algo);

int main(int argc, char *argv[]) {
  int port;
  char *algo;
  if (!check_args(argc, argv, &port, &algo)) {
    return 1;
  }
  printf("Port: %d\n", port);
  printf("Algorithm: %s\n", algo);
  return 0;
}
