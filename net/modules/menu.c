#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../helpers.h"
#include "menu.h"
#include "sender.h"

void send_message(Config* config, int destination, char* message_text) {
  Message message;
  message.type = 1;
  message.source = config->router.id;
  message.destination = destination;

  if (message.destination < 1 || message.destination >= ROUTER_COUNT) {
    printf("%s Invalid destination router.\n", INFO_PREFIX);
    return;
  }

  if (config->router.id != message.destination && config->links[message.destination].router == NULL) {
    printf("%s No link to Router %d.\n", INFO_PREFIX, message.destination);
    return;
  }

  if (message.source == message.destination) {
    printf("%s Source and destination cannot be the same.\n", INFO_PREFIX);
    return;
  }

  strncpy(message.payload, message_text, PAYLOAD_CHAR_LIMIT - 1);
  message.payload[PAYLOAD_CHAR_LIMIT - 1] = '\0';

  sender_put_message(config, message);
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
        strcpy(message, token + 1);
        
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
        strcpy(message, token);
      }
      found_m = 1;
    }
  }
  
  return found_t && found_m;
}

void cleanup(Config* config) {
  pthread_cancel(config->sender.thread_id);
  pthread_cancel(config->packet_handler.thread_id);
  pthread_cancel(config->receiver_thread_id);
  
  pthread_join(config->sender.thread_id, NULL);
  pthread_join(config->packet_handler.thread_id, NULL);
  pthread_join(config->receiver_thread_id, NULL);

  pthread_mutex_destroy(&config->sender.mutex);
  pthread_mutex_destroy(&config->packet_handler.mutex);

  sem_destroy(&config->sender.semaphore);
  sem_destroy(&config->packet_handler.semaphore);

  close(config->socket_fd);

  free(config);
}

void print_menu() {
  printf("\nAvailable commands:\n");
  printf("menu - show this help\n");
  printf("send -t <destination> -m '<message>' - send a message\n");
  printf("exit - exit the program\n");
  printf("\n-------------------------------\n\n");
}

void menu(Config* config) {
  char input[256];
  int destination;
  char message[PAYLOAD_CHAR_LIMIT];
  
  // sleep 10ms for the threads to print messages
  sleep(0.1);
  print_menu();

  while (1) {
    if (fgets(input, sizeof(input), stdin) == NULL) {
      break;
    }
    
    input[strcspn(input, "\n")] = 0;
    
    if (strcmp(input, "menu") == 0) {
      print_menu();
    } else if (strncmp(input, "send", 4) == 0) {
      if (parse_send_command(input, &destination, message)) {
        send_message(config, destination, message);
      } else {
        printf("%s Invalid send command format. Use: send -t <destination> -m <message>\n", ERROR_PREFIX);
      }
    } else if (strcmp(input, "exit") == 0) {
      break;
    } else {
      printf("%s Invalid option.\n", INFO_PREFIX);
    }
  }

  cleanup(config);
}