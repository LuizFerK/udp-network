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
  char buffer[BUFLEN];

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  
  while (1) {
    printf("%s Waiting for messages on port %d...\n", LOG_PREFIX, config->router.port);
    
    memset(buffer,'\0', BUFLEN);

    if ((received_bytes = recvfrom(config->socket_fd, buffer, BUFLEN, 0, (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
      die("Error receiving data");
    }
      
    printf("%s Received packet from %s:%d\n", LOG_PREFIX, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    Message message;
    message.type = 1;
    message.destination = config->router.id;
    strcpy(message.payload, buffer);

    for (int i = 0; i < ROUTER_COUNT; i++) {
      if (config->links[i].router == NULL) continue;
      if (strcmp(config->links[i].router->host, inet_ntoa(client_addr.sin_addr)) == 0) {
        message.source = config->links[i].router->id;
        printf("%s Found source: Router %d\n", LOG_PREFIX, message.source);
        break;
      }
    }

    packet_handler_put_message(config, message);
  }
}