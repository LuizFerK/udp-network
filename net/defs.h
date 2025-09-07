#ifndef DEFS_H
#define DEFS_H

#include <pthread.h>
#include <semaphore.h>

#define ROUTER_CONFIG_FILE "config/router.config"
#define LINK_CONFIG_FILE "config/link.config"
#define ROUTER_COUNT 5
#define HOST_CHAR_LIMIT 15
#define PAYLOAD_CHAR_LIMIT 100
#define QUEUE_CAPACITY 10

typedef struct Router {
  int id;
  int port;
  char host[HOST_CHAR_LIMIT];
} Router;

typedef struct {
  Router *router;
  int weight;
} Link;

typedef struct Message {
  // 1 for data message, 2 for control message
  int type;
  int source;
  int destination;
  char payload[PAYLOAD_CHAR_LIMIT];
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

typedef struct Config {
  Router router;
  Link links[ROUTER_COUNT];
  ControlledQueue sender;
  ControlledQueue packet_handler;
  pthread_t receiver_thread_id;
} Config;

#endif