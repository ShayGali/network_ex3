#include <stdio.h>

#include "RUDP_API.h"

int main(int argc, char* argv[]) {
  printf("~~~~~~~~ RUDP Receiver ~~~~~~~~\n");
  int port;
  int socket =
      RUDP_socket(argv[1], port);  // !!!! צריך לתקן, אוליי להוסיף parse_args
  if (socket == -1) {
    printf("Could not create socket\n");
    return -1;
  }
  printf("Socket created\n");

  FILE* file = fopen("stats", "w+");
  if (file == NULL) {
    printf("Error opening  stats file!\n");
    return 1;
  }
  fprintf(file, "\n\n~~~~~~~~ Statistics ~~~~~~~~\n");
  double average_time = 0;
  double average_speed = 0;

  char* date_received;
  char* total_date;
  int rval = 0;
  int run = 1;
  do {
    rval = RUDP_receive(socket, date_received, sizeof(date_received));
    if (rval == -1) {
      printf("Error receiving data\n");
      return -1;
    }
    memcpy(total_date + strlen(total_date), date_received,
           strlen(date_received));
    if (strnlne(date_received) < 1024) {
      printf("Received date: %s\n", total_date);
    }
  } while (rval == 0);

  return 0;
}
