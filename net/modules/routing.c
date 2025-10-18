#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "sender.h"
#include "routing.h"

#define LOG_PREFIX "[Routing]"

void build_distance_vector(Config* config, int distance_vector[ROUTER_COUNT]) {
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->links[i].router == NULL || config->links[i].expires_at < time(NULL)) {
      distance_vector[i] = -1;
    } else {
      distance_vector[i] = config->links[i].weight;
    }
  }
  distance_vector[config->router.id] = 0;
}

void send_distance_vector(Config* config, int reason) {
  int distance_vector[ROUTER_COUNT];
  build_distance_vector(config, distance_vector);

  Message message;
  message.type = 2;
  message.source = config->router.id;
  memcpy(message.payload, distance_vector, ROUTER_COUNT * sizeof(int));
  
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->links[i].router == NULL) continue;
    message.destination = i;
    message.next_hop = config->routing.routing_table[i];
    message.hops = 1;
    sender_put_message(config, message);
  }

  char* reason_str = reason == 1 ? "TIMEOUT" : "UPDATE";
  printf("%s Sending distance vector to neighbors (%s): ", LOG_PREFIX, reason_str);
  for (int i = 1; i < ROUTER_COUNT; i++) {
    printf("%d ", distance_vector[i]);
  }
  printf("\n");
}

void* routing(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    send_distance_vector(config, 1);
    sleep(config->routing.timeout);
  }
}