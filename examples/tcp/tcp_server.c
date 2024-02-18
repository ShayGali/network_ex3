#include <stdio.h>

// Linux and other UNIXes
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 5060  // The port that the server listens
#define BUFFER_SIZE 1024

int main() {
  // signal(SIGPIPE, SIG_IGN);  // on linux to prevent crash on closing socket

  // Open the listening (server) socket
  // 0 means default protocol for stream sockets (Equivalently, IPPROTO_TCP)
  int listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listeningSocket == -1) {
    printf("Could not create listening socket : %d", errno);
    return 1;
  }

  // Reuse the address if the server socket on was closed
  // and remains for 45 seconds in TIME-WAIT state till the final removal.
  //
  int enableReuse = 1;
  int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse,
                       sizeof(int));
  if (ret < 0) {
    printf("setsockopt() failed with error code : %d", errno);
    return 1;
  }

  // "sockaddr_in" is the "derived" from sockaddr structure
  // used for IPv4 communication. For IPv6, use sockaddr_in6
  //
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));  // fill with zeros

  serverAddress.sin_family = AF_INET;  // IPv4
  // any IP at this port (Address to accept any incoming messages)
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  // htons - convert the port number in host byte order to network byte order
  serverAddress.sin_port = htons(SERVER_PORT);

  // Bind the socket to the port with any IP at this port
  int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress,
                        sizeof(serverAddress));
  if (bindResult == -1) {
    printf("Bind failed with error code : %d", errno);
    // close the socket
    close(listeningSocket);
    return -1;
  }

  printf("Bind() success\n");

  // Make the socket listening; actually mother of all client sockets.
  // 500 is a Maximum size of queue connection requests
  // number of concurrent connections
  int listenResult = listen(listeningSocket, 3);
  if (listenResult == -1) {
    printf("listen() failed with error code : %d", errno);
    // close the socket
    close(listeningSocket);
    return -1;
  }

  // Accept and incoming connection
  printf("Waiting for incoming TCP-connections...\n");
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen = sizeof(clientAddress);

  while (1) {  // main loop to accept clients
    memset(&clientAddress, 0, sizeof(clientAddress));  // fill with zeros
    clientAddressLen = sizeof(clientAddress);
    // accept() will block until a client connects to the server
    int clientSocket = accept(
        listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);

    if (clientSocket == -1) {  // if accept failed
      printf("listen failed with error code : %d", errno);
      // close the sockets
      close(listeningSocket);
      return -1;
    }

    printf("A new client connection accepted\n");

    // Receive a message from client
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == -1) {
      printf("recv failed with error code : %d", errno);
      // close the sockets
      close(listeningSocket);
      close(clientSocket);
      return -1;
    }

    printf("Received: %s\n", buffer);

    // Reply to client
    char *message =
        "Welcome to our TCP-server. your message was received. we got:\n";
    // Reply to client
    char replyBuffer[BUFFER_SIZE + strlen(message) + 1];
    strcpy(replyBuffer, message);
    strcat(replyBuffer, buffer);

    int messageLen = strlen(replyBuffer) + 1;

    // Send the message to the client
    int bytesSent = send(clientSocket, replyBuffer, messageLen, 0);
    if (bytesSent == -1) {
      printf("send() failed with error code : %d", errno);
      close(listeningSocket);
      close(clientSocket);
      return -1;
    } else if (bytesSent == 0) {
      printf("peer has closed the TCP connection prior to send().\n");
    } else if (bytesSent < messageLen) {
      printf("sent only %d bytes from the required %d.\n", messageLen,
             bytesSent);
    } else {
      printf("message was successfully sent.\n");
    }
  }

  close(listeningSocket);

  return 0;
}
