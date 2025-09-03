#include <stdio.h>
#include "../helpers.h"
#include "menu.h"

void send_message(Router router) {
  Message message;
  message.type = 1;
  message.source = router.id;

  printf("\nEnter destination router: ");
  get_int_option(&message.destination);

  if (message.destination < 1 || message.destination >= ROUTER_COUNT) {
    printf("\nInvalid destination router.\n");
    return;
  }

  if (router.id == message.destination) {
    printf("\nSource and destination cannot be the same.\n");
    return;
  }

  if (router.links[message.destination].router == NULL) {
    printf("\nNo link to Router %d.\n", message.destination);
    return;
  }

  printf("\nEnter message: ");
  get_string_option(message.payload);

  printf("\nSending message from Router %d to Router %d...\n", message.source, message.destination);
  printf("Message: %s\n", message.payload);
}


void menu(Router router) {
  int menu_opt = -1;
  while (menu_opt != 2) {
    if (menu_opt != -1 && menu_opt != 1) {
      printf("\nInvalid option.\n");
    }

    if (menu_opt == 1) {
      send_message(router);
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
}