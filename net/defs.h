#ifndef DEFS_H
#define DEFS_H

#define ROUTER_CONFIG_FILE "config/router.config"
#define LINK_CONFIG_FILE "config/link.config"
#define ROUTER_COUNT 5
#define HOST_CHAR_LIMIT 15
#define MESSAGE_CHAR_LIMIT 100

typedef struct Router Router;

typedef struct {
  Router *router;
  int weight;
} Link;

typedef struct Router {
  int id;
  int port;
  char host[HOST_CHAR_LIMIT];
  Link links[ROUTER_COUNT];
} Router;

#endif