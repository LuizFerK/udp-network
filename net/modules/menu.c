#include <stdio.h>
#include "../helpers.h"
#include "menu.h"

void send_message(Config* config) {
  Message message;
  message.type = 1;
  message.source = config->router.id;

  printf("\nEnter destination router: ");
  get_int_option(&message.destination);

  if (message.destination < 1 || message.destination >= ROUTER_COUNT) {
    printf("\nInvalid destination router.\n");
    return;
  }

  if (config->router.id != message.destination && config->links[message.destination].router == NULL) {
    printf("\nNo link to Router %d.\n", message.destination);
    return;
  }

  printf("\nEnter message: ");
  get_string_option(message.payload);

  printf("\nSending message from Router %d to Router %d...\n", message.source, message.destination);
  printf("Message: %s\n", message.payload);
}

void cleanup(Config* config) {
  pthread_cancel(config->sender.thread_id);
  pthread_cancel(config->receiver.thread_id);
  pthread_cancel(config->packetHandler.thread_id);
  
  pthread_join(config->sender.thread_id, NULL);
  pthread_join(config->receiver.thread_id, NULL);
  pthread_join(config->packetHandler.thread_id, NULL);
  
  pthread_mutex_destroy(&config->sender.mutex);
  pthread_mutex_destroy(&config->receiver.mutex);
  pthread_mutex_destroy(&config->packetHandler.mutex);
  
  sem_destroy(&config->sender.semaphore);
  sem_destroy(&config->receiver.semaphore);
  sem_destroy(&config->packetHandler.semaphore);
}

void menu(Config* config) {
  int menu_opt = -1;
  while (menu_opt != 2) {
    if (menu_opt != -1 && menu_opt != 1) {
      printf("\nInvalid option.\n");
    }

    if (menu_opt == 1) {
      send_message(config);
    }

    printf("\nMenu:\n");
    printf("\n1. Send message");
    printf("\n2. Exit\n");
    printf("\nEnter choice: ");
    
    get_int_option(&menu_opt);
    if (menu_opt == -1) {
      printf("\nInvalid option.\n");
    }
  }

  cleanup(config);
}