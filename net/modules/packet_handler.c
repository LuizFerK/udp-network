#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include "packet_handler.h"
#include "routing.h"
#include "../helpers.h"
#include "../ncurses.h"

#define LOG_PREFIX "[Packet Handler]"

void* packet_handler(void* arg) {
  Config* config = (Config*)arg;
  log_message(LOG_PREFIX, "Waiting for messages to handle...");

  while (1) {
    sem_wait(&config->packet_handler.semaphore);

    pthread_mutex_lock(&config->packet_handler.mutex);
    Message message = config->packet_handler.queue.messages[config->packet_handler.queue.front];
    config->packet_handler.queue.front = (config->packet_handler.queue.front + 1) % QUEUE_CAPACITY;
    config->packet_handler.queue.size--;
    pthread_mutex_unlock(&config->packet_handler.mutex);

    char* message_type = message.type == 1 ? "message" : "control message";
    log_message(LOG_PREFIX, "Received %s from Router %d", message_type, message.source);
    
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

void handle_message(Message message) {
  log_message(LOG_PREFIX, "Message: %s", message.payload);
}

void handle_control_message(Config* config, Message message) {
  char vector_str[128] = "";
  int* distance_vector = (int*)message.payload;
  
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (distance_vector[i] == (int)INFINITY) {
      strcat(vector_str, "∞ ");
      continue;
    }
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d ", distance_vector[i]);
    strcat(vector_str, num_str);
  }
  
  log_message(LOG_PREFIX, "Distance vector: %s", vector_str);
  
  time_t now = time(NULL);
  config->links[message.source].expires_at = now + (config->routing.timeout * 3);
  memcpy(config->links[message.source].distance_vector, distance_vector, ROUTER_COUNT * sizeof(int));

  if (update_routing_data(config) == 1) {
    send_distance_vector(config, 2);
  }
}
