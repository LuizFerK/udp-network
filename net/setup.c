#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "modules/packet_handler.h"
#include "modules/receiver.h"
#include "modules/sender.h"
#include "defs.h"
#include "setup.h"

void setup_routers(Router routers[ROUTER_COUNT]) {
  FILE *router_file = fopen(ROUTER_CONFIG_FILE, "r");
  if (router_file == NULL) {
    fprintf(stderr, "Error while opening router config file.\n");
    exit(1);
  }
  
  int router_id, router_port;
  char router_host[HOST_CHAR_LIMIT];
  while (fscanf(router_file, "%d %d %s", &router_id, &router_port, router_host) == 3) {
    routers[router_id].id = router_id;
    routers[router_id].port = router_port;
    strcpy(routers[router_id].host, router_host);
  }

  fclose(router_file);
}

void setup_thread(Config* config, pthread_t* thread_id, void* (*func)(void*)) {
  if (pthread_create(thread_id, NULL, func, config) != 0) {
    fprintf(stderr, "Error while creating thread.\n");
    exit(1);
  }
}

void setup_controlled_queue(Config* config, ControlledQueue* queue, void* (*func)(void*)) {
  pthread_mutex_init(&queue->mutex, NULL);
  sem_init(&queue->semaphore, 0, 0);
  queue->queue.front = 0;
  queue->queue.rear = 0;
  queue->queue.size = 0;

  setup_thread(config, &queue->thread_id, func);
}

void setup_links(int id, Link links[ROUTER_COUNT], Router routers[ROUTER_COUNT]) {
  FILE *link_file = fopen(LINK_CONFIG_FILE, "r");
  if (link_file == NULL) {
    fprintf(stderr, "Error while opening link config file.\n");
    exit(1);
  }

  int source, dest, weight;
  while (fscanf(link_file, "%d %d %d", &source, &dest, &weight) == 3) {
    if (source == id) {
      links[dest].router = &routers[dest];
      links[dest].weight = weight;
    }

    if (dest == id) {
      links[source].router = &routers[source];
      links[source].weight = weight;
    }
  }

  fclose(link_file);

  for (int i = 0; i < ROUTER_COUNT; i++) {
    if (links[i].router == NULL) continue;
    printf("-> Linked to Router %d: Host: %s, Port: %d, Weight: %d\n",
           links[i].router->id,
           links[i].router->host,
           links[i].router->port,
           links[i].weight);
  }

  printf("\n");
}

Config* setup(int id) {
  Config* config = malloc(sizeof(Config));
  if (config == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for config.\n");
    exit(1);
  }
  
  Router* routers = malloc(sizeof(Router) * ROUTER_COUNT);
  if (routers == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for routers.\n");
    exit(1);
  }

  if (id == -1) {
    fprintf(stderr, "Error: ID is required. Use -i <id>\n");
    fprintf(stderr, "Usage: ./net.o -i <id>\n");
    exit(1);
  }

  if (id <= 0 || id >= ROUTER_COUNT) {
    fprintf(stderr, "Error: Invalid router ID.\n");
    exit(1);
  }
  
  setup_routers(routers);

  config->router = routers[id];
  printf("Connected to Router %d: Host: %s, Port: %d\n\n",
         config->router.id,
         config->router.host,
         config->router.port);

  setup_links(id, config->links, routers);

  setup_controlled_queue(config, &config->sender, sender);
  setup_thread(config, &config->receiver_thread_id, receiver);
  setup_controlled_queue(config, &config->packet_handler, packet_handler);

  return config;
}