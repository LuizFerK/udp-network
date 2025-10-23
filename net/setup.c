#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "modules/packet_handler.h"
#include "modules/receiver.h"
#include "modules/sender.h"
#include "modules/routing.h"
#include "defs.h"
#include "setup.h"
#include "ncurses.h"

// Initialize router configuration and start all threads
Config* setup(int id, int routing_timeout) {
  // Allocate memory for main configuration structure
  Config* config = malloc(sizeof(Config));
  if (config == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for config.\n");
    exit(1);
  }
  
  // Allocate memory for router information array
  Router* routers = malloc(sizeof(Router) * ROUTER_COUNT);
  if (routers == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for routers.\n");
    exit(1);
  }

  // Validate router ID parameter
  if (id == -1) {
    fprintf(stderr, "Error: ID is required. Use -i <id>\n");
    fprintf(stderr, "Usage: ./net.o -i <id>\n");
    exit(1);
  }

  if (id <= 0 || id >= ROUTER_COUNT) {
    fprintf(stderr, "Error: Invalid router ID.\n");
    exit(1);
  }
  
  // Load router information from configuration file
  setup_routers(routers);

  // Set this router's information
  config->router = routers[id];
  
  // Initialize ncurses UI
  init_ncurses();

  log_info("Connected to Router %d: Host: %s, Port: %d",
            config->router.id,
            config->router.host,
            config->router.port);

  // Setup network topology links
  setup_links(id, config->links, routers);

  // Calculate and display graph diameter
  set_infinity_with_graph_diameter(config);

  // Create and bind UDP socket
  setup_udp_socket(config);

  // Initialize thread-safe queues and start worker threads
  setup_controlled_queue(config, &config->sender, sender);
  setup_thread(config, &config->receiver_thread_id, receiver);
  setup_controlled_queue(config, &config->packet_handler, packet_handler);

  // Configure routing algorithm and start routing thread
  config->routing.timeout = routing_timeout;
  setup_thread(config, &config->routing.thread_id, routing);
  
  // Display help menu
  print_menu_help();

  return config;
}

// Load router information from configuration file
void setup_routers(Router routers[ROUTER_COUNT]) {
  FILE *router_file = fopen(ROUTER_CONFIG_FILE, "r");
  if (router_file == NULL) {
    fprintf(stderr, "Error while opening router config file.\n");
    exit(1);
  }
  
  // Parse router configuration: ID, port, host
  int router_id, router_port;
  char router_host[HOST_CHAR_LIMIT];
  while (fscanf(router_file, "%d %d %s", &router_id, &router_port, router_host) == 3) {
    routers[router_id].id = router_id;
    routers[router_id].port = router_port;
    strcpy(routers[router_id].host, router_host);
  }

  fclose(router_file);
}

// Setup network topology links from configuration file
void setup_links(int id, Link links[ROUTER_COUNT], Router routers[ROUTER_COUNT]) {
  FILE *link_file = fopen(LINK_CONFIG_FILE, "r");
  if (link_file == NULL) {
    fprintf(stderr, "Error while opening link config file.\n");
    exit(1);
  }

  // Parse link configuration: source, destination, weight
  int source, dest, weight;
  while (fscanf(link_file, "%d %d %d", &source, &dest, &weight) == 3) {
    // Setup outgoing links (this router as source)
    if (source == id) {
      links[dest].router = &routers[dest];
      links[dest].weight = weight;
    }

    // Setup incoming links (this router as destination)
    if (dest == id) {
      links[source].router = &routers[source];
      links[source].weight = weight;
    }
  }

  fclose(link_file);

  // Display connected routers
  for (int i = 1; i < ROUTER_COUNT; i++) {
    if (links[i].router == NULL) continue;
    log_info("-> Linked to Router %d: Host: %s, Port: %d, Weight: %d",
             links[i].router->id,
             links[i].router->host,
             links[i].router->port,
             links[i].weight);
  }
}

// Calculate and print the diameter of the network graph
void set_infinity_with_graph_diameter(Config* config) {
  // Initialize distance matrix with infinity values
  int dist[ROUTER_COUNT][ROUTER_COUNT];
  const int INF = 999999; // Use a large integer instead of floating-point INFINITY
  
  // Initialize distance matrix
  for (int i = 1; i < ROUTER_COUNT; i++) {
    for (int j = 1; j < ROUTER_COUNT; j++) {
      if (i == j) {
        dist[i][j] = 0; // Distance to self is 0
      } else {
        dist[i][j] = INF; // Initialize with infinity
      }
    }
  }
  
  // Read complete network topology from links.config file
  FILE *link_file = fopen(LINK_CONFIG_FILE, "r");
  if (link_file == NULL) {
    fprintf(stderr, "Error while opening link config file for diameter calculation.\n");
    exit(1);
  }

  // Parse link configuration and set direct link weights
  int source, dest, weight;
  while (fscanf(link_file, "%d %d %d", &source, &dest, &weight) == 3) {
    // Set bidirectional link weights (undirected graph)
    dist[source][dest] = weight;
    dist[dest][source] = weight;
  }

  fclose(link_file);
  
  // Floyd-Warshall algorithm to find shortest paths
  for (int k = 1; k < ROUTER_COUNT; k++) {
    for (int i = 1; i < ROUTER_COUNT; i++) {
      for (int j = 1; j < ROUTER_COUNT; j++) {
        if (dist[i][k] + dist[k][j] < dist[i][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
        }
      }
    }
  }
  
  // Find the diameter (maximum shortest path)
  int diameter = 0;
  for (int i = 1; i < ROUTER_COUNT; i++) {
    for (int j = 1; j < ROUTER_COUNT; j++) {
      if (i != j && dist[i][j] != INF && dist[i][j] > diameter) {
        diameter = dist[i][j];
      }
    }
  }
  
  // Print the diameter
  log_info("Graph diameter: %d", diameter);
  config->infinity = diameter + 1;
}

// Create and bind UDP socket for network communication
void setup_udp_socket(Config* config) {
  // Create UDP socket
  config->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (config->socket_fd == -1) {
    fprintf(stderr, "Error creating socket.\n");
    exit(1);
  }
    
  // Configure socket address
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(config->router.port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
  // Bind socket to router's port
  if( bind(config->socket_fd , (struct sockaddr*)&server_addr, sizeof(server_addr) ) == -1) {
    fprintf(stderr, "Error binding socket to port.\n");
    exit(1);
  }
}

// Initialize thread-safe queue and start worker thread
void setup_controlled_queue(Config* config, ControlledQueue* queue, void* (*func)(void*)) {
  // Initialize synchronization primitives
  pthread_mutex_init(&queue->mutex, NULL);
  sem_init(&queue->semaphore, 0, 0);
  
  // Initialize queue state
  queue->queue.front = 0;
  queue->queue.rear = 0;
  queue->queue.size = 0;

  // Start worker thread for this queue
  setup_thread(config, &queue->thread_id, func);
}

// Create and start a new thread
void setup_thread(Config* config, pthread_t* thread_id, void* (*func)(void*)) {
  if (pthread_create(thread_id, NULL, func, config) != 0) {
    fprintf(stderr, "Error while creating thread.\n");
    exit(1);
  }
}