#include <pthread.h>
#include "defs.h"

#define LOG_BUFFER_SIZE 1000
#define LOG_WINDOW_HEIGHT (LINES - 2) * 2 / 3
#define MENU_WINDOW_HEIGHT (LINES - 2) - LOG_WINDOW_HEIGHT

typedef struct UI {
    WINDOW* log_window;
    WINDOW* menu_window;
    pthread_mutex_t ui_mutex;
    int log_count;
    int log_start;
    char log_buffer[LOG_BUFFER_SIZE][1024];
  } UI;

void init_ncurses();
void log_message(const char* prefix, const char* format, ...);
void log_info(const char* format, ...);
void log_error(const char* format, ...);
void print_menu_help();
int get_user_input(char* input, int max_len);
void cleanup_ncurses();