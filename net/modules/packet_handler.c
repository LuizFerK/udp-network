#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include "packet_handler.h"

#define LOG_PREFIX "[Packet Handler]"

void handle_message(Message message) {
  printf("%s Message: %s\n", LOG_PREFIX, message.payload);
}

void handle_control_message(Config* config, Message message) {
  printf("%s Distance vector: ", LOG_PREFIX);
  int* distance_vector = (int*)message.payload;
  for (int i = 1; i < ROUTER_COUNT; i++) {
    printf("%d ", distance_vector[i]);
  }
  
  time_t now = time(NULL);
  config->links[message.source].expires_at = now + (config->routing.timeout * 3);
  memcpy(config->links[message.source].distance_vector, distance_vector, ROUTER_COUNT * sizeof(int));
  
  printf("\n");
}

void* packet_handler(void* arg) {
  Config* config = (Config*)arg;
  printf("%s Waiting for messages to handle...\n", LOG_PREFIX);

  while (1) {
    sem_wait(&config->packet_handler.semaphore);

    pthread_mutex_lock(&config->packet_handler.mutex);
    Message message = config->packet_handler.queue.messages[config->packet_handler.queue.front];
    config->packet_handler.queue.front = (config->packet_handler.queue.front + 1) % QUEUE_CAPACITY;
    config->packet_handler.queue.size--;
    pthread_mutex_unlock(&config->packet_handler.mutex);

    char* message_type = message.type == 1 ? "message" : "control message";
    printf("%s Received %s from Router %d\n", LOG_PREFIX, message_type, message.source);
    message.type == 1 ? handle_message(message) : handle_control_message(config, message);
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