#define WINDOW_MAX_SIZE 1024  // CHECK

/*
we will keep two variables to keep track of the window size
1. base: the sequence number of the first byte in the window
2. next: If the sender sent’s the packet, the variable next is incremented to 1
*/

/*
the timeouts will be handled using timers
*/

/*
Packets are stored in a queue and each is assigned a value.
This value represents the sum of the system's current time and the network delay

When the current time of the system is associated with the assigned value the
packet is freed from the queue and sent over the network.

While the packet is sent over the network it gives an (ACK) acknowledgment
*/