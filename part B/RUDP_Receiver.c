#include <stdio.h>
#include <time.h>

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
  clock_t start, end;
  char* date_received;
  char* total_date = "";
  int rval = 0;
  int run = 1;
  start = clock();
  end = clock();
  do {
    rval = RUDP_receive(socket, date_received, sizeof(date_received));
    if (rval == -1) {
      printf("Error receiving data\n");
      return -1;
    }
    if (rval == 1 && start < end) {
      start = clock();
    }
    if (rval == 1) {
      strcat(total_date, date_received);
    }
    if (rval == 2) {
      strcat(total_date, date_received);
      printf("Received date: %s\n", sizeof(total_date));
      end = clock();
      double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
      average_time += time_taken;
      double speed = 2 / time_taken;
      average_speed += speed;
      fprintf(file, "Run #%d Data: Time=%f S ; Speed=%f MB/S\n", run,
              time_taken, speed);
      run++;
    }
  } while (rval >= 0);
  if (rval == -2) {
    printf("connection closed\n");
    // add the average time and speed to the file
    fprintf(file, "\n");
    fprintf(file, "Average time: %f S\n", average_time / (run - 1));
    fprintf(file, "Average speed: %f MB/S\n", average_speed / (run - 1));

    // end the file with nice message
    fprintf(file, "\n\n-----------------------------\n");
    fprintf(file, "Thank you for using our RUDP service\n");
    rewind(file);
    char print_buffer[100];
    while (fgets(print_buffer, 100, file) != NULL) {
      printf("%s", print_buffer);
    }
  }

  // Close the file
  fclose(file);
  remove("stats");

  return 0;
}
