#include <stdio.h>
#include "../helpers.h"
#include "menu.h"

void send_message(Router router) {
  int destination;
  char message[MESSAGE_CHAR_LIMIT];

  printf("\nEnter destination router: ");
  get_int_option(&destination);

  if (destination < 1 || destination >= ROUTER_COUNT) {
    printf("\nInvalid destination router.\n");
    return;
  }

  if (router.id == destination) {
    printf("\nSource and destination cannot be the same.\n");
    return;
  }

  if (router.links[destination].router == NULL) {
    printf("\nNo link to Router %d.\n", destination);
    return;
  }

  printf("\nEnter message: ");
  get_string_option(message);

  printf("\nSending message from Router %d to Router %d...\n", router.id, destination);
  printf("Message: %s\n", message);
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