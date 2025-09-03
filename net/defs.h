#ifndef DEFS_H
#define DEFS_H

#define ROUTER_CONFIG_FILE "config/router.config"
#define LINK_CONFIG_FILE "config/link.config"
#define ROUTER_COUNT 5
#define HOST_CHAR_LIMIT 15
#define PAYLOAD_CHAR_LIMIT 100

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

typedef struct Message {
  // 1 for data message, 2 for control message
  int type;
  int source;
  int destination;
  char payload[PAYLOAD_CHAR_LIMIT];
} Message;

#endif