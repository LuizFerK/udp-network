#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "setup.h"

Router setup(int id) {
  Router routers[ROUTER_COUNT];
  
  if (id == -1) {
    fprintf(stderr, "Error: ID is required. Use -i <id>\n");
    fprintf(stderr, "Usage: ./net.o -i <id>\n");
    exit(1);
  }

  if (id <= 0 || id >= ROUTER_COUNT) {
    fprintf(stderr, "Error: Invalid router ID.\n");
    exit(1);
  }
  
  for (int i = 0; i < ROUTER_COUNT; i++) {
    routers[i].id = 0;
    routers[i].port = 0;
    routers[i].host[0] = '\0';
    for (int j = 0; j < ROUTER_COUNT; j++) {
      routers[i].links[j].router = NULL;
      routers[i].links[j].weight = 0;
    }
  }

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

  Router router = routers[id];
  
  printf("Connected to Router %d: Host: %s, Port: %d\n", router.id, router.host, router.port);

  fclose(router_file);

  FILE *link_file = fopen(LINK_CONFIG_FILE, "r");
  if (link_file == NULL) {
    fprintf(stderr, "Error while opening link config file.\n");
    exit(1);
  }

  int source, dest, weight;
  while (fscanf(link_file, "%d %d %d", &source, &dest, &weight) == 3) {
    if (source == router.id) {
      router.links[dest].router = &routers[dest];
      router.links[dest].weight = weight;
    }

    if (dest == router.id) {
      router.links[source].router = &routers[source];
      router.links[source].weight = weight;
    }
  }

  fclose(link_file);

  for (int i = 0; i < ROUTER_COUNT; i++) {
    if (router.links[i].router == NULL) continue;
    printf("-> Linked to Router %d: Host: %s, Port: %d, Weight: %d\n", router.links[i].router->id, router.links[i].router->host, router.links[i].router->port, router.links[i].weight);
  }

  return router;
}