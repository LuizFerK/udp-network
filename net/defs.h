#ifndef DEFS_H
#define DEFS_H

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>

#define ROUTER_CONFIG_FILE "config/router.config"
#define LINK_CONFIG_FILE "config/link.config"
#define ROUTER_COUNT 5
#define HOST_CHAR_LIMIT 15
#define PAYLOAD_SIZE 100 // should be greater than ROUTER_COUNT * sizeof(int) to handle the distance vector
#define QUEUE_CAPACITY 10
#define INFO_PREFIX "[INFO]"
#define ERROR_PREFIX "[ERROR]"

typedef struct Router {
  int id;
  int port;
  char host[HOST_CHAR_LIMIT];
} Router;

typedef struct {
  int distance_vector[ROUTER_COUNT];
  time_t expires_at;
  Router *router;
  int weight;
} Link;

typedef struct Message {
  // 1 for data message, 2 for control message
  int type;
  int source;
  int destination;
  int next_hop;
  int hops;
  char payload[PAYLOAD_SIZE];
} Message;

typedef struct Queue {
  Message messages[QUEUE_CAPACITY];
  int front;
  int rear;
  int size;
} Queue;

typedef struct ControlledQueue {
  pthread_mutex_t mutex;
  sem_t semaphore;
  Queue queue;
  pthread_t thread_id;
} ControlledQueue;

typedef struct Routing {
  int routing_table[ROUTER_COUNT];
  int last_distance_vector[ROUTER_COUNT];
  pthread_t thread_id;
  int timeout;
} Routing;

typedef struct Config {
  Router router;
  Routing routing;
  Link links[ROUTER_COUNT];
  ControlledQueue sender;
  ControlledQueue packet_handler;
  pthread_t receiver_thread_id;
  int socket_fd;
} Config;

#endif