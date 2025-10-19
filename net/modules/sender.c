#include <semaphore.h>
#include <stdio.h>
#include "sender.h"

#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<unistd.h> //close()
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define LOG_PREFIX "[Sender]"
#define BUFLEN 512

static void die(char *s) {
  perror(s);
  exit(1);
}

void* sender(void* arg) {
  Config* config = (Config*)arg;

  struct sockaddr_in next_hop_addr;
  int next_hop_addr_len=sizeof(next_hop_addr);

  next_hop_addr.sin_family = AF_INET;

  printf("%s Waiting for messages to send...\n", LOG_PREFIX);
  
  while (1) {
    sem_wait(&config->sender.semaphore);

    pthread_mutex_lock(&config->sender.mutex);
    Message message = config->sender.queue.messages[config->sender.queue.front];
    config->sender.queue.front = (config->sender.queue.front + 1) % QUEUE_CAPACITY;
    config->sender.queue.size--;
    pthread_mutex_unlock(&config->sender.mutex);

    next_hop_addr.sin_port = htons(config->links[message.next_hop].router->port);

    if (inet_pton(AF_INET, config->links[message.next_hop].router->host, &next_hop_addr.sin_addr) <= 0) {
      die("Error converting host to IP address");
    }

    if (sendto(config->socket_fd, &message, sizeof(Message) , 0 , (struct sockaddr *) &next_hop_addr, next_hop_addr_len) == -1) {
      die("Error sending message");
    }

    char* message_type = message.type == 1 ? "message" : "control message";
    printf("%s Sent %s to Router %d\n", LOG_PREFIX, message_type, message.next_hop);
  }
}

void sender_put_message(Config* config, Message message) {
  pthread_mutex_lock(&config->sender.mutex);
  config->sender.queue.messages[config->sender.queue.rear] = message;
  config->sender.queue.rear = (config->sender.queue.rear + 1) % QUEUE_CAPACITY;
  config->sender.queue.size++;
  pthread_mutex_unlock(&config->sender.mutex);
  sem_post(&config->sender.semaphore);
}