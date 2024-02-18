#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Define server IP and port
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5060
#define BUFFER_SIZE 1024

int main() {
  // Create a socket - IPv4, TCP, default protocol
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  // Check if socket creation was successful
  if (sock == -1) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Define server address
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;  // IPv4
  // Convert port to network byte order
  server_addr.sin_port = htons(SERVER_PORT);
  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) <= 0) {
    perror("invalid address or address not supported");
    exit(EXIT_FAILURE);
  }

  // Connect to the server - get the socket, server address and server address
  // length
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("connection failed");
    exit(EXIT_FAILURE);
  }

  // Print success message
  printf("Connected to server\n");

  // Send data to the server
  char message[] = "How are you?";
  int bytes_sent = send(sock, message, strlen(message), 0);
  if (bytes_sent == -1) {
    perror("send failed");
    exit(EXIT_FAILURE);
  } else if (bytes_sent == 0) {
    printf("peer has closed the connection\n");
  } else if (bytes_sent < strlen(message)) {
    printf("sent only %d bytes of %ld\n", bytes_sent, strlen(message));
  } else {
    printf("sent %d bytes\n", bytes_sent);
  }

  // Receive response from the server
  char response[BUFFER_SIZE] = {0};
  // the recv gets the socket, the buffer to store the received data, the size
  // of the buffer and the flags - 0 for no flags
  int bytes_received = recv(sock, response, sizeof(response) - 1, 0);
  if (bytes_received == -1) {
    perror("receive failed");
    exit(EXIT_FAILURE);
  } else if (bytes_received == 0) {
    printf("peer has closed the connection\n");
  } else {
    printf("received %d bytes\n", bytes_received);
  }
  response[bytes_received] = '\0';  // add \0 to the end of the string

  // Print the received response
  printf("Received response from server: %s\n", response);

  // Close the socket
  close(sock);
  return 0;
}