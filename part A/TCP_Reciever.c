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

int main(int argc, char *argv[]) {
  int port;
  char *algo;
  if (!parse_args(argc, argv, &port, &algo)) {
    return 1;
  }
  
  printf("Port: %d\n", port);
  printf("Algorithm: %s\n", algo);

  /*
  ~ template code ~
  socket = create_socket();
  // wait for a connection
  client_socket = accept_connection(socket);
  do{
    file = receive_file(socket);
    time = measure_time(file); // need to send the file?
    get_client_decision(client_socket);
  }while(1);
  close_connection(client_socket);
  print_logs();
  calc_stats();
  close_socket(socket);
  */
  return 0;
}
