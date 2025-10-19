#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>

// Window dimensions and layout
#define LOG_BUFFER_SIZE 1000

// Thread-safe logging structure
typedef struct {
    WINDOW* log_window;
    WINDOW* menu_window;
    char log_buffer[LOG_BUFFER_SIZE][1024];
    int log_count;
    int log_start;
    pthread_mutex_t log_mutex;
    int initialized;
    int log_window_height;
    int menu_window_height;
} NCursesUI;

// Global UI instance
extern NCursesUI* ui;

// Function declarations
int init_ncurses();
void cleanup_ncurses();
void log_message(const char* prefix, const char* format, ...);
void log_info(const char* format, ...);
void log_error(const char* format, ...);
void refresh_menu_window();
void refresh_log_window();
void print_menu_help();
int get_user_input(char* input, int max_len);
