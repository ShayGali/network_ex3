#include "args_parser.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
int check_args(int argc, char *argv[], int *port, char **algo) {
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