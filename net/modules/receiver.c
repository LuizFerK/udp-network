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

  struct sockaddr_in server_addr, client_addr;
     
  int socket_fd, received_bytes;
  socklen_t client_addr_len = sizeof(client_addr);
  char buffer[BUFLEN];
    
  if ((socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("Error creating socket");
  }
    
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(config->router.port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
  if( bind(socket_fd , (struct sockaddr*)&server_addr, sizeof(server_addr) ) == -1) {
    die("Error binding socket to port");
  }
    
  while (1) {
    printf("%s Waiting for messages on port %d...\n", LOG_PREFIX, config->router.port);
    
    memset(buffer,'\0', BUFLEN);

    if ((received_bytes = recvfrom(socket_fd, buffer, BUFLEN, 0, (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
      die("Error receiving data");
    }
      
    printf("%s Received packet from %s:%d\n", LOG_PREFIX, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    Message message;
    message.type = 1;
    // message.source = ?;
    // message.destination = ?;
    strcpy(message.payload, buffer);

    packet_handler_put_message(config, message);
  }
}