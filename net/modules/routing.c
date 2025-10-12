#include <unistd.h>
#include <stdio.h>
#include "routing.h"

#define LOG_PREFIX "[Routing]"

void send_distance_vector(Config* config, int reason) {
  printf("%s Sending distance vector to neighbors", LOG_PREFIX);

  if (reason == 1) {
    printf(" (TIMEOUT)\n");
  } else if (reason == 2) {
    printf(" (UPDATE)\n");
  }

  // TODO: send distance vector to neighbors
}

void* routing(void* arg) {
  Config* config = (Config*)arg;

  while (1) {
    send_distance_vector(config, 1);
    printf("%s Waiting %d seconds to send distance vector...\n", LOG_PREFIX, config->routing.timeout);
    sleep(config->routing.timeout);
  }
}