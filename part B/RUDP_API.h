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
  char data[1024];
} RUDP;

/**
 * Creating a RUDP socket and creating a handshake between
two peers.
*/
int RUDP_socket(char *ip, int port);

/**
 * opening a connection between two peers.
 *
 * הפונקציה תשלח הודעה עם
 * SYN
 * ותחכה לקבלת הודעה עם
 * SYN + ACK
 *
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

/**
 * clculating the checksum of the packet.
 */
int checksum(RUDP *packet);
