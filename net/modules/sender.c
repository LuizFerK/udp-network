#include <semaphore.h>
#include <stdio.h>
#include "receiver.h"
#include "sender.h"

#define LOG_PREFIX "[Sender]"

void* sender(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    printf("%s Waiting for messages to send...\n", LOG_PREFIX);
    sem_wait(&config->sender.semaphore);

    pthread_mutex_lock(&config->sender.mutex);
    Message message = config->sender.queue.messages[config->sender.queue.front];
    config->sender.queue.front = (config->sender.queue.front + 1) % QUEUE_CAPACITY;
    config->sender.queue.size--;
    pthread_mutex_unlock(&config->sender.mutex);

    if (message.source == message.destination) {
      receiver_put_message(config, message);
    } else {
      // send message through udp
    }

    printf("\n%s Sent message to Router %d\n", LOG_PREFIX, message.destination);
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