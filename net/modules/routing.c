#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "sender.h"
#include "routing.h"

#define LOG_PREFIX "[Routing]"

void build_distance_vector(Config* config, int distance_vector[ROUTER_COUNT]) {
  for (int i = 0; i < ROUTER_COUNT; i++) {
    if (config->links[i].router == NULL) {
      // distance_vector[i] = INFINITY;
      distance_vector[i] = -1;
    } else {
      distance_vector[i] = config->links[i].weight;
    }
  }
}

void send_distance_vector(Config* config, int reason) {
  int distance_vector[ROUTER_COUNT];
  build_distance_vector(config, distance_vector);

  Message message;
  message.type = 2;
  message.source = config->router.id;
  memcpy(message.payload, distance_vector, ROUTER_COUNT * sizeof(int));
  
  for (int i = 0; i < ROUTER_COUNT; i++) {
    if (config->links[i].router == NULL) continue;
    message.destination = i;
    sender_put_message(config, message);
  }

  char* reason_str = reason == 1 ? "TIMEOUT" : "UPDATE";
  printf("%s Sending distance vector to neighbors (%s): ", LOG_PREFIX, reason_str);
  for (int i = 0; i < ROUTER_COUNT; i++) {
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