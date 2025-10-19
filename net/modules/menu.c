#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../helpers.h"
#include "../ncurses.h"
#include "menu.h"
#include "sender.h"

void menu(Config* config) {
  char input[256];
  int destination;
  char message[PAYLOAD_SIZE];
  
  while (1) {
    if (get_user_input(input, sizeof(input)) != 0) {
      break;
    }
    
    if (strncmp(input, "send", 4) == 0) {
      if (parse_send_command(input, &destination, message)) {
        send_message(config, destination, message);
      } else {
        log_error("Invalid send command format. Use: send -t <destination> -m <message>");
      }
    } else if (strcmp(input, "status") == 0) {
      print_router_status(config);
    } else if (strcmp(input, "exit") == 0) {
      break;
    } else {
      log_info("Invalid option.");
    }
  }
}

void send_message(Config* config, int destination, char* message_text) {
  Message message;
  message.type = 1;
  message.source = config->router.id;
  message.destination = destination;

  if (message.destination < 1 || message.destination >= ROUTER_COUNT) {
    log_info("Invalid destination router.");
    return;
  }

  if (message.source == message.destination) {
    log_info("Source and destination cannot be the same.");
    return;
  }
  
  message.next_hop = config->routing.routing_table[destination];
  message.hops = 1;

  if (message.next_hop == (int)INFINITY) {
    log_info("Router %d is unreachable.", destination);
    return;
  }

  strncpy(message.payload, message_text, PAYLOAD_SIZE - 1);
  message.payload[PAYLOAD_SIZE - 1] = '\0';

  sender_put_message(config, message);
}

void print_router_status(Config* config) {
  char routing_table_str[128] = "";
  char distance_vector_str[128] = "";
  char self_distance_vector_str[128] = "";
  
  // Routing table
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->routing.routing_table[i] == (int)INFINITY) {
      strcat(routing_table_str, "∞ ");
      continue;
    }
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d ", config->routing.routing_table[i]);
    strcat(routing_table_str, num_str);
  }

  // Last calculated distance vector
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->routing.last_distance_vector[i] == (int)INFINITY) {
      strcat(distance_vector_str, "∞ ");
      continue;
    }
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d ", config->routing.last_distance_vector[i]);
    strcat(distance_vector_str, num_str);
  }

  // Self distance vector
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (i == config->router.id) {
      strcat(self_distance_vector_str, "0 ");
    } else if (config->links[i].router == NULL || (config->links[i].expires_at == 0 || config->links[i].expires_at < time(NULL))) {
      strcat(self_distance_vector_str, "∞ ");
    } else {
      char num_str[16];
      snprintf(num_str, sizeof(num_str), "%d ", config->links[i].weight);
      strcat(self_distance_vector_str, num_str);
    }
  }

  log_info("Router %d status:", config->router.id);
  log_info("Routing table: %s", routing_table_str);
  log_info("Last calculated distance vector: %s", distance_vector_str);
  log_info("Self distance vector: %s", self_distance_vector_str);

  // Distance vectors from neighbors
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (config->links[i].expires_at == 0 || config->links[i].expires_at < time(NULL)) continue;
    char distance_vector_str[128] = "";

    for (int j = 1; j < ROUTER_COUNT; j++) {
      if (config->links[i].distance_vector[j] == (int)INFINITY) {
        strcat(distance_vector_str, "∞ ");
        continue;
      }
      char num_str[16];
      snprintf(num_str, sizeof(num_str), "%d ", config->links[i].distance_vector[j]);
      strcat(distance_vector_str, num_str);
    }
    log_info("Distance vector from Router %d: %s", i, distance_vector_str);
  }
}

int parse_send_command(char* input, int* destination, char* message) {
  char* token;
  char* saveptr;
  int found_t = 0, found_m = 0;
  
  token = strtok_r(input, " ", &saveptr);
  
  if (strcmp(token, "send") != 0) {
    return 0;
  }
  
  while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
    if (strcmp(token, "-t") == 0) {
      token = strtok_r(NULL, " ", &saveptr);
      if (token == NULL) return 0;
      *destination = atoi(token);
      found_t = 1;
    } else if (strcmp(token, "-m") == 0) {
      token = strtok_r(NULL, " ", &saveptr);
      if (token == NULL) return 0;
      
      // supports quoted messages to handle spaces in the message
      if (token[0] == '\'') {
        strncpy(message, token + 1, PAYLOAD_SIZE);
        
        while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
          if (token[strlen(token) - 1] == '\'') {
            token[strlen(token) - 1] = '\0';
            strcat(message, " ");
            strcat(message, token);
            break;
          } else {
            strcat(message, " ");
            strcat(message, token);
          }
        }
      } else {
        strncpy(message, token, PAYLOAD_SIZE);
      }
      found_m = 1;
    }
  }
  
  return found_t && found_m;
}
