#ifndef DEFS_H
#define DEFS_H

#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>
#include <time.h>

// Configuration file paths
#define ROUTER_CONFIG_FILE "config/router.config"
#define LINK_CONFIG_FILE "config/link.config"

// Network topology constants
#define ROUTER_COUNT 5
#define HOST_CHAR_LIMIT 15
#define PAYLOAD_SIZE 100 // should be greater than ROUTER_COUNT * sizeof(int) to handle the distance vector
#define QUEUE_CAPACITY 10

// Logging prefixes
#define INFO_PREFIX "[INFO]"
#define ERROR_PREFIX "[ERROR]"

// Router information structure
typedef struct Router {
  int id;
  int port;
  char host[HOST_CHAR_LIMIT];
} Router;

// Link information between routers with distance vector and timeout
typedef struct {
  int distance_vector[ROUTER_COUNT]; // Distance vector received from this neighbor
  time_t expires_at;                // When this link expires (for timeout detection)
  Router *router;                    // Pointer to neighbor router
  int weight;                        // Link weight/cost
} Link;

// Network message structure for data and control messages
typedef struct Message {
  int type;           // 1 for data message, 2 for control message
  int source;         // Source router ID
  int destination;    // Destination router ID
  int next_hop;      // Next hop router for routing
  int hops;          // Number of hops taken
  char payload[PAYLOAD_SIZE]; // Message content or distance vector data
} Message;

// Circular queue for message storage
typedef struct Queue {
  Message messages[QUEUE_CAPACITY];
  int front;  // Front of queue
  int rear;   // Rear of queue
  int size;   // Current number of messages
} Queue;

// Thread-safe queue with synchronization primitives
typedef struct ControlledQueue {
  pthread_mutex_t mutex;    // Protects queue operations
  sem_t semaphore;          // Signals when messages are available
  Queue queue;              // The actual message queue
  pthread_t thread_id;      // Thread that processes this queue
} ControlledQueue;

// Routing information and distance vector algorithm state
typedef struct Routing {
  int routing_table[ROUTER_COUNT];        // Next hop for each destination
  int last_distance_vector[ROUTER_COUNT]; // Last calculated distance vector
  pthread_t thread_id;                    // Routing algorithm thread
  int timeout;                            // Routing update interval
} Routing;

// Main configuration structure containing all router state
typedef struct Config {
  Router router;                    // This router's information
  Routing routing;                 // Routing algorithm state
  Link links[ROUTER_COUNT];        // Links to other routers
  ControlledQueue sender;          // Outgoing message queue
  ControlledQueue packet_handler;  // Incoming message processing queue
  pthread_t receiver_thread_id;    // UDP receiver thread
  int socket_fd;                   // UDP socket file descriptor
  int infinity;                    // Infinity value
} Config;

#endif