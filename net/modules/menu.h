#include "../defs.h"

void send_message(Config* config, int destination, char* message_text);
int parse_send_command(char* input, int* destination, char* message);
void menu(Config* config);