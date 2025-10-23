#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "sender.h"
#include "routing.h"
#include "../helpers.h"
#include "../ncurses.h"

#define LOG_PREFIX "[Routing]"

// Routing thread: periodically updates routing table and sends distance vector to neighbors
void* routing(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    // Update routing table using Bellman-Ford algorithm
    update_routing_data(config);
    
    // Send distance vector to all neighbors
    send_distance_vector(config, 1);
    
    // Wait for next update interval
    sleep(config->routing.timeout);
  }
}

// Send distance vector to all neighbor routers
void send_distance_vector(Config* config, int reason) {
  Message message;
  message.type = 2;  // Control message
  message.source = config->router.id;
  
  // Copy current distance vector to message payload
  memcpy(message.payload, config->routing.last_distance_vector, ROUTER_COUNT * sizeof(int));
  
  // Send to all connected neighbors
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->links[i].router == NULL) continue;
    
    message.destination = i;
    message.next_hop = config->links[i].router->id;
    message.hops = 1;
    sender_put_message(config, message);
  }

  // Log the control message with reason and distance vector
  char* reason_str = reason == 1 ? "TIMEOUT" : "UPDATE";
  char log_msg[256] = "Sending distance vector to neighbors (";
  strcat(log_msg, reason_str);
  strcat(log_msg, "): ");
  
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->routing.last_distance_vector[i] == config->infinity) {
      strcat(log_msg, "∞ ");
      continue;
    }
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d ", config->routing.last_distance_vector[i]);
    strcat(log_msg, num_str);
  }
  log_message(LOG_PREFIX, log_msg);
}
