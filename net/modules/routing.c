#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "sender.h"
#include "routing.h"
#include "../helpers.h"

#define LOG_PREFIX "[Routing]"

void send_distance_vector(Config* config, int reason) {
  Message message;
  message.type = 2;
  message.source = config->router.id;
  memcpy(message.payload, config->routing.last_distance_vector, ROUTER_COUNT * sizeof(int));
  
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->links[i].router == NULL) continue;
    
    message.destination = i;
    message.next_hop = config->links[i].router->id;
    message.hops = 1;
    sender_put_message(config, message);
  }

  char* reason_str = reason == 1 ? "TIMEOUT" : "UPDATE";
  printf("%s Sending distance vector to neighbors (%s): ", LOG_PREFIX, reason_str);
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->routing.last_distance_vector[i] == (int)INFINITY) {
      printf("∞ ");
      continue;
    }
    printf("%d ", config->routing.last_distance_vector[i]);
  }
  printf("\n");
}

void* routing(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    update_routing_data(config);
    send_distance_vector(config, 1);
    sleep(config->routing.timeout);
  }
}