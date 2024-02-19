/*The Sender will send a randomly generated file*/

#include <stdio.h>

#include "args_parser.h"
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
