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
  // Parse command line arguments for router ID and routing timeout
  int id = get_cli_opt(argc, argv, "-i", -1);
  int routing_timeout = get_cli_opt(argc, argv, "-t", 10);

  // Initialize router configuration and start all threads
  Config* config = setup(id, routing_timeout);
  
  // Start interactive menu for user commands
  menu(config);
  
  // Clean up resources and terminate threads
  cleanup(config);

  return 0;
}

// Clean up all resources and terminate threads gracefully
void cleanup(Config* config) {
  // Cancel all running threads
  pthread_cancel(config->sender.thread_id);
  pthread_cancel(config->packet_handler.thread_id);
  pthread_cancel(config->receiver_thread_id);
  
  // Wait for threads to finish
  pthread_join(config->sender.thread_id, NULL);
  pthread_join(config->packet_handler.thread_id, NULL);
  pthread_join(config->receiver_thread_id, NULL);

  // Destroy synchronization primitives
  pthread_mutex_destroy(&config->sender.mutex);
  pthread_mutex_destroy(&config->packet_handler.mutex);

  sem_destroy(&config->sender.semaphore);
  sem_destroy(&config->packet_handler.semaphore);

  // Close UDP socket
  close(config->socket_fd);

  // Clean up ncurses UI and free memory
  cleanup_ncurses();
  free(config);
}
