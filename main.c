#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROUTER_CONFIG_FILE "config/router.config"
#define LINK_CONFIG_FILE "config/link.config"
#define HOST_CHAR_LIMIT 15
#define ROUTER_COUNT 5
#define LINK_COUNT_LIMIT ROUTER_COUNT

typedef struct Router Router;

typedef struct {
  Router *router;
  int weight;
} Link;

typedef struct Router {
  int id;
  int port;
  char host[HOST_CHAR_LIMIT];
  Link links[LINK_COUNT_LIMIT];
} Router;

int main(int argc, char *argv[]) {
  Router routers[ROUTER_COUNT];
  Router router_config;
  
  for (int i = 0; i < ROUTER_COUNT; i++) {
    routers[i].id = 0;
    routers[i].port = 0;
    routers[i].host[0] = '\0';
    for (int j = 0; j < LINK_COUNT_LIMIT; j++) {
      routers[i].links[j].router = NULL;
      routers[i].links[j].weight = 0;
    }
  }
  
  router_config.id = -1;
  router_config.port = 0;
  router_config.host[0] = '\0';
  for (int i = 0; i < LINK_COUNT_LIMIT; i++) {
    router_config.links[i].router = NULL;
    router_config.links[i].weight = 0;
  }
  
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      router_config.id = atoi(argv[i + 1]);
      break;
    }
  }
  
  if (router_config.id == -1) {
    fprintf(stderr, "Error: ID is required. Use -i <id>\n");
    fprintf(stderr, "Usage: %s -i <id>\n", argv[0]);
    return 1;
  }

  if (router_config.id <= 0 || router_config.id >= ROUTER_COUNT) {
    fprintf(stderr, "Error: Invalid router ID.\n");
    return 1;
  }

  FILE *router_file = fopen(ROUTER_CONFIG_FILE, "r");
  if (router_file == NULL) {
    fprintf(stderr, "Error while opening router config file.\n");
    return 1;
  }
  
  int router_id, router_port;
  char router_host[HOST_CHAR_LIMIT];
  while (fscanf(router_file, "%d %d %s", &router_id, &router_port, router_host) == 3) {
    routers[router_id].id = router_id;
    routers[router_id].port = router_port;
    strcpy(routers[router_id].host, router_host);
  }

  router_config = routers[router_config.id];
  
  printf("Router ID: %d, Port: %d, Host: %s\n", router_config.id, router_config.port, router_config.host);

  fclose(router_file);

  FILE *link_file = fopen(LINK_CONFIG_FILE, "r");
  if (link_file == NULL) {
    fprintf(stderr, "Error while opening link config file.\n");
    return 1;
  }

  int source, dest, weight;
  while (fscanf(link_file, "%d %d %d", &source, &dest, &weight) == 3) {
    if (source == router_config.id) {
      router_config.links[dest].router = &routers[dest];
      router_config.links[dest].weight = weight;
    }

    if (dest == router_config.id) {
      router_config.links[source].router = &routers[source];
      router_config.links[source].weight = weight;
    }
  }

  fclose(link_file);

  for (int i = 0; i < LINK_COUNT_LIMIT; i++) {
    if (router_config.links[i].router == NULL) continue;
    printf("Link %d: %s:%d, Weight: %d\n", router_config.links[i].router->id, router_config.links[i].router->host, router_config.links[i].router->port, router_config.links[i].weight);
  }

  return 0;
}
