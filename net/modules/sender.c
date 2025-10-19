#include <semaphore.h>
#include <stdio.h>
#include "sender.h"
#include "../ncurses.h"

#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<unistd.h> //close()
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define LOG_PREFIX "[Sender]"

// Sender thread: processes outgoing messages from the sender queue
void* sender(void* arg) {
  Config* config = (Config*)arg;

  struct sockaddr_in next_hop_addr;
  int next_hop_addr_len=sizeof(next_hop_addr);

  next_hop_addr.sin_family = AF_INET;

  log_message(LOG_PREFIX, "Waiting for messages to send...");
  
  while (1) {
    // Wait for messages to be available in the queue
    sem_wait(&config->sender.semaphore);

    // Get message from queue (thread-safe)
    pthread_mutex_lock(&config->sender.mutex);
    Message message = config->sender.queue.messages[config->sender.queue.front];
    config->sender.queue.front = (config->sender.queue.front + 1) % QUEUE_CAPACITY;
    config->sender.queue.size--;
    pthread_mutex_unlock(&config->sender.mutex);

    // Configure destination address
    next_hop_addr.sin_port = htons(config->links[message.next_hop].router->port);

    // Convert hostname to IP address
    if (inet_pton(AF_INET, config->links[message.next_hop].router->host, &next_hop_addr.sin_addr) <= 0) {
      perror("Error converting host to IP address");
      exit(1);
    }

    // Send message via UDP
    if (sendto(config->socket_fd, &message, sizeof(Message) , 0 , (struct sockaddr *) &next_hop_addr, next_hop_addr_len) == -1) {
      perror("Error sending message");
      exit(1);
    }

    // Log successful transmission
    char* message_type = message.type == 1 ? "message" : "control message";
    log_message(LOG_PREFIX, "Sent %s to Router %d", message_type, message.next_hop);
  }
}

// Add message to sender queue for transmission
void sender_put_message(Config* config, Message message) {
  // Thread-safe queue insertion
  pthread_mutex_lock(&config->sender.mutex);
  config->sender.queue.messages[config->sender.queue.rear] = message;
  config->sender.queue.rear = (config->sender.queue.rear + 1) % QUEUE_CAPACITY;
  config->sender.queue.size++;
  pthread_mutex_unlock(&config->sender.mutex);
  
  // Signal sender thread that a message is available
  sem_post(&config->sender.semaphore);
}