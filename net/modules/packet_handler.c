#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include "packet_handler.h"
#include "routing.h"
#include "sender.h"
#include "../helpers.h"
#include "../ncurses.h"

#define LOG_PREFIX "[Packet Handler]"

// Packet handler thread: processes incoming messages and routes them
void* packet_handler(void* arg) {
  Config* config = (Config*)arg;
  log_message(LOG_PREFIX, "Waiting for messages to handle...");

  while (1) {
    // Wait for messages to be available in the queue
    sem_wait(&config->packet_handler.semaphore);

    // Get message from queue (thread-safe)
    pthread_mutex_lock(&config->packet_handler.mutex);
    Message message = config->packet_handler.queue.messages[config->packet_handler.queue.front];
    config->packet_handler.queue.front = (config->packet_handler.queue.front + 1) % QUEUE_CAPACITY;
    config->packet_handler.queue.size--;
    pthread_mutex_unlock(&config->packet_handler.mutex);

    // Check if message is for this router
    if (message.destination == config->router.id) {
      char* message_type = message.type == 1 ? "message" : "control message";
      log_message(LOG_PREFIX, "Received %s from Router %d", message_type, message.source);
      
      // Handle data messages or control messages (distance vectors)
      message.type == 1 ? handle_message(message) : handle_control_message(config, message);
      continue;
    }

    // Message needs to be forwarded - get next hop from routing table
    int next_hop = config->routing.routing_table[message.destination];
    if (next_hop == config->infinity) {
      log_message(LOG_PREFIX, "Next hop router is unreachable.");
      continue;
    }

    log_message(LOG_PREFIX, "Forwarding message to Router %d", next_hop);

    // Update message for forwarding
    message.next_hop = next_hop;
    message.hops = message.hops + 1;
    sender_put_message(config, message);
  }
}

// Add message to packet handler queue for processing
void packet_handler_put_message(Config* config, Message message) {
  // Thread-safe queue insertion
  pthread_mutex_lock(&config->packet_handler.mutex);
  config->packet_handler.queue.messages[config->packet_handler.queue.rear] = message;
  config->packet_handler.queue.rear = (config->packet_handler.queue.rear + 1) % QUEUE_CAPACITY;
  config->packet_handler.queue.size++;
  pthread_mutex_unlock(&config->packet_handler.mutex);
  
  // Signal packet handler thread that a message is available
  sem_post(&config->packet_handler.semaphore);
}

// Handle data messages (user messages)
void handle_message(Message message) {
  log_message(LOG_PREFIX, "Message: %s", message.payload);
}

// Handle control messages (distance vector updates)
void handle_control_message(Config* config, Message message) {
  char vector_str[128] = "";
  int* distance_vector = (int*)message.payload;
  
  // Build distance vector string for logging
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (distance_vector[i] == config->infinity) {
      strcat(vector_str, "∞ ");
      continue;
    }
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d ", distance_vector[i]);
    strcat(vector_str, num_str);
  }
  
  log_message(LOG_PREFIX, "Distance vector: %s", vector_str);
  
  // Update link information and extend timeout
  time_t now = time(NULL);
  config->links[message.source].expires_at = now + (config->routing.timeout * 3);
  memcpy(config->links[message.source].distance_vector, distance_vector, ROUTER_COUNT * sizeof(int));

  // Update routing table and send distance vector to neighbors if changed
  if (update_routing_data(config) == 1) {
    send_distance_vector(config, 2);
  }
}
