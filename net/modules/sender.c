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

  struct sockaddr_in si_other;
  int s, slen=sizeof(si_other);
  char buf[BUFLEN];

  if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("socket");
  }

  memset((char *) &si_other, 0, sizeof(si_other));

  while (1) {
    printf("%s Waiting for messages to send...\n", LOG_PREFIX);
    sem_wait(&config->sender.semaphore);

    pthread_mutex_lock(&config->sender.mutex);
    Message message = config->sender.queue.messages[config->sender.queue.front];
    config->sender.queue.front = (config->sender.queue.front + 1) % QUEUE_CAPACITY;
    config->sender.queue.size--;
    pthread_mutex_unlock(&config->sender.mutex);

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(config->links[message.destination].router->port);

    printf("Destination router: %d\n", message.destination);
    printf("Destination router: %d\n", config->links[message.destination].router->id);
    printf("%s Sending message to %s:%d\n", LOG_PREFIX, config->links[message.destination].router->host, config->links[message.destination].router->port);
      
    if (inet_pton(AF_INET, config->links[message.destination].router->host, &si_other.sin_addr) <= 0) {
      fprintf(stderr, "inet_pton() failed\n");
      exit(1);
    }

    // send message through udp
    printf("\n%s Sent message to Router %d\n", LOG_PREFIX, message.destination);

    //send the message
    if (sendto(s, message.payload, strlen(message.payload) , 0 , (struct sockaddr *) &si_other, slen)==-1) {
      die("sendto()");
    }
     
    //receive a reply and print it
    //clear the buffer by filling null, it might have previously received data
    memset(buf,'\0', BUFLEN);
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