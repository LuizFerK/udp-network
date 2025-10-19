#include <arpa/inet.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "packet_handler.h"
#include "receiver.h"
#include "../ncurses.h"

#define LOG_PREFIX "[Receiver]"

// Receiver thread: listens for incoming UDP messages
void* receiver(void* arg) {
  Config* config = (Config*)arg;
    
  int received_bytes;
  Message message;

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  
  log_message(LOG_PREFIX, "Waiting for messages on port %d...", config->router.port);
  
  while (1) {
    // Clear message buffer
    memset(&message, 0, sizeof(Message));

    // Receive UDP message
    if ((received_bytes = recvfrom(config->socket_fd, &message, sizeof(Message), 0, (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
      perror("Error receiving data");
      exit(1);
    }
      
    // Log received message details
    log_message(LOG_PREFIX, "Received packet from %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    log_message(LOG_PREFIX, "Message type: %d, source: %d, destination: %d", message.type, message.source, message.destination);

    // Forward message to packet handler for processing
    packet_handler_put_message(config, message);
  }
}
