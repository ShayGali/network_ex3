
#define RETRY 3
#define WINDOW_MAX_SIZE 1024

#define SUCCESS 1
#define FAILURE 0
#define ERROR -1

typedef struct RUDP_flags {
  unsigned int SYN : 1;
  unsigned int ACK : 1;
  unsigned int DATA : 1;
  unsigned int FIN : 1;
} RUDP_flags;

typedef struct _RUDP {
  RUDP_flags flags;
  int seq_num;
  int checksum;
  int length;  // the length of the data
  char data[WINDOW_MAX_SIZE];
} RUDP;

/**
 * Creating a RUDP socket
 * @return the socket number if success, -1 if failed.
 */
int RUDP_socket();

/**
 * opening a connection between two peers.
 * (a connection is established with 2 way handshake - SYN, SYN-ACK)
 *
 * @param socket - the socket to connect to.
 * @return 1 is success, 0 if failed.
 */
int RUDP_connect(int socket, char *ip, int port);

/**
 * Listening for incoming connection requests.
 * (a connection is established with 2 way handshake - SYN, SYN-ACK)
 *
 * @param socket - the socket to listen to.
 * @return 1 is success, 0 if failed.
 */
int RUDP_get_connection(int socket, int port);

/**
 * Sending data to the peer.
 *  The function should wait for an acknowledgment packet, and if it didn’t
 * receive any, retransmits the data.
 *
 * @param socket - the socket to send from.
 * @param data - the data to send.
 * @param data_length - the length of the data.
 * @return 1 is success, 0 if failed.
 */
int RUDP_send(int socket, char *data, int data_length);

/**
 * Receiving data from the peer.
 * will put the received data in the data parameter, and the length of the data
 * in the data_length parameter.
 *
 * @param socket - the socket to receive from.
 * @param data - the data to receive.
 * @param data_length - the length of the data.
 * @return 1 is success, 0 if failed.
 */
int RUDP_receive(int socket, char **data, int *data_length);

/**
 * Closing the RUDP socket.
 */
int RUDP_close(int socket);
