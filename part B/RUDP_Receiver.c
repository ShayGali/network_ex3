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
  char* date_received;
  char* total_date;
  int rval = 0;
  do {
    rval = RUDP_receive(socket, date_received, sizeof(date_received));
    if (rval == -1) {
      printf("Error receiving data\n");
      return -1;
    }
    memcpy(total_date + strlen(total_date), date_received,
           strlen(date_received));
  } while (rval == 0);

  return 0;
}
