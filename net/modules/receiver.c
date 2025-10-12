#include <arpa/inet.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "packet_handler.h"
#include "receiver.h"

#define LOG_PREFIX "[Receiver]"
#define BUFLEN 512

static void die(char *s) {
  perror(s);
  exit(1);
}

void* receiver(void* arg) {
  Config* config = (Config*)arg;
    
  int received_bytes;
  Message message;

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  
  printf("%s Waiting for messages on port %d...\n", LOG_PREFIX, config->router.port);
  
  while (1) {
    memset(&message, 0, sizeof(Message));

    if ((received_bytes = recvfrom(config->socket_fd, &message, sizeof(Message), 0, (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
      die("Error receiving data");
    }
      
    printf("%s Received packet from %s:%d\n", LOG_PREFIX, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    printf("%s Message type: %d, source: %d, destination: %d\n", LOG_PREFIX, message.type, message.source, message.destination);

    packet_handler_put_message(config, message);
  }
}