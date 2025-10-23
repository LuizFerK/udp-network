#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "helpers.h"
#include "ncurses.h"

// Parse command line arguments for specific options
int get_cli_opt(int argc, char *argv[], char *key, int default_value) {
  int value = default_value;
  
  // Search for the key in command line arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], key) == 0 && i + 1 < argc) {
      value = atoi(argv[i + 1]);
      break;
    }
  }

  return value;
}

// Distributed Bellman-Ford algorithm for distance vector routing
int update_routing_data(Config* config) {
  int updated = 0;

  int routing_table[ROUTER_COUNT];
  int distance_vector[ROUTER_COUNT];

  // Initialize routing table and distance vector based on direct neighbor weights
  for (int i = 1; i < ROUTER_COUNT; i++) {
    // Check if link exists and hasn't expired
    if (config->links[i].router == NULL || (config->links[i].expires_at == 0 || config->links[i].expires_at < time(NULL))) {
      routing_table[i] = config->infinity;
      distance_vector[i] = config->infinity;
    } else {
      routing_table[i] = i;  // Direct route through neighbor
      distance_vector[i] = config->links[i].weight;  // Direct link cost
    }
  }

  // Apply Bellman-Ford: update routes using neighbors' distance vectors
  for (int i = 1; i < ROUTER_COUNT; i++) {
    // Skip expired or non-existent links
    if (config->links[i].expires_at == 0 || config->links[i].expires_at < time(NULL)) continue;

    // Check each destination through this neighbor
    for (int j = 1; j < ROUTER_COUNT; j++) {
      if (config->links[i].distance_vector[j] >= config->infinity) continue;

      // Calculate cost: link weight + neighbor's distance to destination
      int weight = config->links[i].weight + config->links[i].distance_vector[j];
      if (weight < distance_vector[j]) {
        distance_vector[j] = weight;
        routing_table[j] = i;  // Route through neighbor i
      }
    }
  }

  // Set self-route (distance 0 to self)
  routing_table[config->router.id] = config->router.id;
  distance_vector[config->router.id] = 0;

  // Check if routing table has changed
  compare_distance_vectors(config->routing.last_distance_vector, distance_vector, &updated);
  
  if (updated) {
    // Update stored routing information
    memcpy(config->routing.last_distance_vector, distance_vector, ROUTER_COUNT * sizeof(int));
    memcpy(config->routing.routing_table, routing_table, ROUTER_COUNT * sizeof(int));

    // Log routing table update with timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), " (%Y-%m-%d %H:%M:%S)", tm_info);
    
    char log_msg[256] = "Updated routing table: ";
    for (int i = 1; i < ROUTER_COUNT; i++) {
      if (routing_table[i] == config->infinity) {
        strcat(log_msg, "∞ ");
        continue;
      }
      char num_str[16];
      snprintf(num_str, sizeof(num_str), "%d ", routing_table[i]);
      strcat(log_msg, num_str);
    }
    strcat(log_msg, timestamp);
    log_info(log_msg);
  }

  return updated;
}

// Compare current distance vector with previous one to detect changes
void compare_distance_vectors(int* last_distance_vector, int* distance_vector, int* updated) {
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (last_distance_vector[i] != distance_vector[i]) {
      *updated = 1;
      break;
    }
  }
}
