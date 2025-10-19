#include "defs.h"

int get_cli_opt(int argc, char *argv[], char *key, int default_value);
void compare_distance_vectors(int* last_distance_vector, int* distance_vector, int* updated);
int update_routing_data(Config* config);
