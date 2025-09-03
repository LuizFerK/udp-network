#include <semaphore.h>
#include <stdio.h>
#include "sender.h"

#define LOG_PREFIX "[Sender]"

void* sender(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    printf("%s Waiting for packets...\n", LOG_PREFIX);
    sem_wait(&config->sender.semaphore);

    // pthread_mutex_lock(&config->sender.mutex);
    // Message message = config->sender.queue.messages[config->sender.queue.front];
    // config->sender.queue.front = (config->sender.queue.front + 1) % QUEUE_CAPACITY;
    // config->sender.queue.size--;
    // pthread_mutex_unlock(&config->sender.mutex);
    // return message;
  }
}