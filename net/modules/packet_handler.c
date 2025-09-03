#include <semaphore.h>
#include <stdio.h>
#include "packet_handler.h"

#define LOG_PREFIX "[Packet Handler]"

void* packet_handler(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    printf("%s Waiting for packets...\n", LOG_PREFIX);
    sem_wait(&config->packetHandler.semaphore);

    // pthread_mutex_lock(&config->packetHandler.mutex);
    // Message message = config->packetHandler.queue.messages[config->packetHandler.queue.front];
    // config->packetHandler.queue.front = (config->packetHandler.queue.front + 1) % QUEUE_CAPACITY;
    // config->packetHandler.queue.size--;
    // pthread_mutex_unlock(&config->packetHandler.mutex);
    // return message;
  }
}