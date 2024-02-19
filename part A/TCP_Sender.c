/*The Sender will send a randomly generated file*/

#include <stdio.h>

#include "args_parser.h"

/**
 * Read the file to be sent
 */
void read_file();
/**
 * Create a TCP socket between the sender and the receiver
 */
void create_socket();

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
  printf("Port: %d\n", port);
  printf("Algorithm: %s\n", algo);

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
  return 0;
}
