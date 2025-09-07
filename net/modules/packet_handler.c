#include <semaphore.h>
#include <stdio.h>
#include "packet_handler.h"

#define LOG_PREFIX "[Packet Handler]"

void* packet_handler(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    printf("%s Waiting for messages...\n", LOG_PREFIX);
    sem_wait(&config->packet_handler.semaphore);

    pthread_mutex_lock(&config->packet_handler.mutex);
    Message message = config->packet_handler.queue.messages[config->packet_handler.queue.front];
    config->packet_handler.queue.front = (config->packet_handler.queue.front + 1) % QUEUE_CAPACITY;
    config->packet_handler.queue.size--;
    pthread_mutex_unlock(&config->packet_handler.mutex);

    printf("\n%s Received message from Router %d\n", LOG_PREFIX, message.source);
  }
}

void packet_handler_put_message(Config* config, Message message) {
  pthread_mutex_lock(&config->packet_handler.mutex);
  config->packet_handler.queue.messages[config->packet_handler.queue.rear] = message;
  config->packet_handler.queue.rear = (config->packet_handler.queue.rear + 1) % QUEUE_CAPACITY;
  config->packet_handler.queue.size++;
  pthread_mutex_unlock(&config->packet_handler.mutex);
  sem_post(&config->packet_handler.semaphore);
}