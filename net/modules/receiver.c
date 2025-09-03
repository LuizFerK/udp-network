#include <semaphore.h>
#include <stdio.h>
#include "receiver.h"

#define LOG_PREFIX "[Receiver]"

void* receiver(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    printf("%s Waiting for packets...\n", LOG_PREFIX);
    sem_wait(&config->receiver.semaphore);

    // pthread_mutex_lock(&config->receiver.mutex);
    // Message message = config->receiver.queue.messages[config->receiver.queue.front];
    // config->receiver.queue.front = (config->receiver.queue.front + 1) % QUEUE_CAPACITY;
    // config->receiver.queue.size--;
    // pthread_mutex_unlock(&config->receiver.mutex);
    // return message;
  }
}