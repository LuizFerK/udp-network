#include "defs.h"

void setup_thread(Config* config, pthread_t* thread_id, void* (*func)(void*));
void setup_controlled_queue(Config* config, ControlledQueue* queue, void* (*func)(void*));
void setup_udp_socket(Config* config);
void setup_links(int id, Link links[ROUTER_COUNT], Router routers[ROUTER_COUNT]);
void setup_routers(Router routers[ROUTER_COUNT]);
Config* setup(int id, int routing_timeout);