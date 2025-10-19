#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "helpers.h"
#include "ncurses.h"

int get_cli_opt(int argc, char *argv[], char *key, int default_value) {
  int value = default_value;
  
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], key) == 0 && i + 1 < argc) {
      value = atoi(argv[i + 1]);
      break;
    }
  }

  return value;
}

int update_routing_data(Config* config) {
  int updated = 0;

  int routing_table[ROUTER_COUNT];
  int distance_vector[ROUTER_COUNT];

  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->links[i].router == NULL || (config->links[i].expires_at == 0 || config->links[i].expires_at < time(NULL))) {
      routing_table[i] = (int)INFINITY;
      distance_vector[i] = (int)INFINITY;
    } else {
      routing_table[i] = i;
      distance_vector[i] = config->links[i].weight;
    }
  }

  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->links[i].expires_at == 0 || config->links[i].expires_at < time(NULL)) continue;

    for (int j = 1; j < ROUTER_COUNT; j++) {
      if (config->links[i].distance_vector[j] == (int)INFINITY) continue;

      int weight = config->links[i].weight + config->links[i].distance_vector[j];
      if (weight < distance_vector[j]) {
        distance_vector[j] = weight;
        routing_table[j] = i;
      }
    }
  }

  routing_table[config->router.id] = config->router.id;
  distance_vector[config->router.id] = 0;

  compare_distance_vectors(config->routing.last_distance_vector, distance_vector, &updated);
  
  if (updated) {
    memcpy(config->routing.last_distance_vector, distance_vector, ROUTER_COUNT * sizeof(int));
    memcpy(config->routing.routing_table, routing_table, ROUTER_COUNT * sizeof(int));

    char log_msg[256] = "Updated routing table: ";
    for (int i = 1; i < ROUTER_COUNT; i++) {
      if (routing_table[i] == (int)INFINITY) {
        strcat(log_msg, "∞ ");
        continue;
      }
      char num_str[16];
      snprintf(num_str, sizeof(num_str), "%d ", routing_table[i]);
      strcat(log_msg, num_str);
    }
    log_info(log_msg);
  }

  return updated;
}

void compare_distance_vectors(int* last_distance_vector, int* distance_vector, int* updated) {
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (last_distance_vector[i] != distance_vector[i]) {
      *updated = 1;
      break;
    }
  }
}
