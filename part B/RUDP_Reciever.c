#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "RUDP_API.h"

int main(int argc, char* argv[]) {
  printf("~~~~~~~~ RUDP Receiver ~~~~~~~~\n");
  int port = atoi(argv[2]);
  int socket = RUDP_socket();
  if (socket == -1) {
    printf("Could not create socket\n");
    return -1;
  }
  printf("Socket created\n");

  int rval = RUDP_get_connection(socket, port);
  if (rval == 0) {
    printf("Could not get connection\n");
    return -1;
  }
  printf("Connection established\n");
  FILE* file = fopen("stats", "w+");
  if (file == NULL) {
    printf("Error opening  stats file!\n");
    return 1;
  }
  fprintf(file, "\n\n~~~~~~~~ Statistics ~~~~~~~~\n");
  double average_time = 0;
  double average_speed = 0;
  clock_t start, end;

  char* date_received = NULL;
  int data_length = 0;
  char* total_date = "";
  rval = 0;
  int run = 1;

  start = clock();
  end = clock();

  do {
    printf("Receiving data\n");
    rval = RUDP_receive(socket, &date_received, &data_length);
    if (rval < 0) {
      printf("Error receiving data\n");
      return -1;
    }
    if (rval == 1 &&
        start <
            end) {  // if the data received is the first one, start the timer
      start = clock();
    }
    if (rval == 1) {  // if the data received is not the last one, add it to the
                      // total data
      strcat(total_date, date_received);
    }
    if (rval == 2) {  // if the data received is the last one, take it and write
                      // the stats it to the file
      strcat(total_date, date_received);
      printf("Received date: %zu\n", sizeof(total_date));
      end = clock();
      double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
      average_time += time_taken;
      double speed = 2 / time_taken;
      average_speed += speed;
      fprintf(file, "Run #%d Data: Time=%f S ; Speed=%f MB/S\n", run,
              time_taken, speed);
      total_date = "";
      run++;
    }
  } while (rval >= 0);  //   keep the loop until the connection is closed

  if (rval ==
      -2) {  // check if the connection was closed by the sender and print stats
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
