#include <string.h>
#include <stdlib.h>

int get_cli_opt(int argc, char *argv[], char *key) {
  int id = -1;
  
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], key) == 0 && i + 1 < argc) {
      id = atoi(argv[i + 1]);
      break;
    }
  }

  return id;
}