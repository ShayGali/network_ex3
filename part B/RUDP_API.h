
#define RETRY 3
#define WINDOW_MAX_SIZE 1024

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
  int length;
  char data[WINDOW_MAX_SIZE];
} RUDP;

/**
 * Creating a RUDP socket and creating a handshake between two peers.
 */
int RUDP_socket(char *ip, int port);

/**
 * opening a connection between two peers.
 * (a connection is established with 2 way handshake - SYN, SYN-ACK)
 *
 * @param socket - the socket to connect to.
 * @return 1 is success, 0 if failed.
 */
int RUDP_connect(int socket);

/**
 * Sending data to the peer.
 *  The function should wait for an acknowledgment packet, and if it didn’t
 * receive any, retransmits the data.
 */
int RUDP_send(int socket, char *data, int data_length);

/**
 * Receiving data from the peer.
 */
int RUDP_receive(int socket, char *data, int data_length);

/**
 * Closing the RUDP socket.
 */
int RUDP_close(int socket);
