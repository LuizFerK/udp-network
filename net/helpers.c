#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int get_cli_opt(int argc, char *argv[], char *key, int default_value) {
  int value = default_value;
  
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], key) == 0 && i + 1 < argc) {
      value = atoi(argv[i + 1]);
      break;
    }
  }

  return value;
}

void get_int_option(int* option) {
  if (scanf("%d", option) != 1) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    *option = -1;
  }
}

void get_string_option(char* option) {
  if (scanf("%s", option) != 1) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    *option = '\0';
  }
}
