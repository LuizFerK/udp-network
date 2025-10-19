#include "../defs.h"

void handle_message(Message message);
void handle_control_message(Config* config, Message message);
void* packet_handler(void* arg);
void packet_handler_put_message(Config* config, Message message);