#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "net/defs.h"
#include "net/helpers.h"
#include "net/setup.h"
#include "net/modules/menu.h"
#include "net/ncurses.h"

void cleanup(Config* config);

int main(int argc, char *argv[]) {
  int id = get_cli_opt(argc, argv, "-i", -1);
  int routing_timeout = get_cli_opt(argc, argv, "-t", 10);

  Config* config = setup(id, routing_timeout);
  
  menu(config);
  cleanup(config);

  return 0;
}

void cleanup(Config* config) {
  pthread_cancel(config->sender.thread_id);
  pthread_cancel(config->packet_handler.thread_id);
  pthread_cancel(config->receiver_thread_id);
  
  pthread_join(config->sender.thread_id, NULL);
  pthread_join(config->packet_handler.thread_id, NULL);
  pthread_join(config->receiver_thread_id, NULL);

  pthread_mutex_destroy(&config->sender.mutex);
  pthread_mutex_destroy(&config->packet_handler.mutex);

  sem_destroy(&config->sender.semaphore);
  sem_destroy(&config->packet_handler.semaphore);

  close(config->socket_fd);

  cleanup_ncurses();
  free(config);
}
