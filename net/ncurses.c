#include "ncurses.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <stdarg.h>

UI* ui = NULL;

// Initialize ncurses interface with log and menu windows
void init_ncurses() {
  // Set up locale and ncurses environment
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  // Allocate UI structure
  ui = malloc(sizeof(UI));
  if (ui == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for UI.\n");
    exit(1);
  }
      
  // Initialize UI mutex for thread safety
  pthread_mutex_init(&ui->ui_mutex, NULL);
  
  // Create log window (top) and menu window (bottom)
  ui->log_window = newwin(LOG_WINDOW_HEIGHT - 2, COLS - 2, 1, 1);
  ui->menu_window = newwin(MENU_WINDOW_HEIGHT - 2, COLS - 2, LOG_WINDOW_HEIGHT + 1, 1);
  
  // Draw window borders and labels
  box(stdscr, 0, 0);
  mvhline(LOG_WINDOW_HEIGHT, 0, ACS_HLINE, COLS);
  mvprintw(0, 2, " LOGS ");
  mvprintw(LOG_WINDOW_HEIGHT, 2, " MENU ");
  refresh();
}

// Thread-safe logging function with circular buffer
void log_message(const char* prefix, const char* format, ...) {
    char formatted_message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(formatted_message, sizeof(formatted_message), format, args);
    va_end(args);
    
    // Thread-safe access to UI
    pthread_mutex_lock(&ui->ui_mutex);
    
    // Add message to circular buffer
    snprintf(ui->log_buffer[ui->log_count], sizeof(ui->log_buffer[ui->log_count]), 
             "%s %s", prefix, formatted_message);
    
    // Update circular buffer pointers
    ui->log_count = (ui->log_count + 1) % LOG_BUFFER_SIZE;
    if (ui->log_count == ui->log_start) {
        ui->log_start = (ui->log_start + 1) % LOG_BUFFER_SIZE;
    }
    
    // Clear and redraw log window
    werase(ui->log_window);
    
    // Calculate total entries in buffer
    int total_entries = 0;
    if (ui->log_count >= ui->log_start) {
        total_entries = ui->log_count - ui->log_start;
    } else {
        total_entries = LOG_BUFFER_SIZE - ui->log_start + ui->log_count;
    }
    
    // Display the most recent entries that fit in the window
    int max_display = LOG_WINDOW_HEIGHT - 2; // Account for border
    int start_display = (total_entries > max_display) ? (total_entries - max_display) : 0;
    int y = 0;
    
    for (int i = start_display; i < total_entries && y < max_display; i++) {
        int idx = (ui->log_start + i) % LOG_BUFFER_SIZE;
        mvwprintw(ui->log_window, y, 0, "%s", ui->log_buffer[idx]);
        y++;
    }
    
    wrefresh(ui->log_window);
    pthread_mutex_unlock(&ui->ui_mutex);
}

// Log info messages with [INFO] prefix
void log_info(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char formatted_message[512];
  vsnprintf(formatted_message, sizeof(formatted_message), format, args);
  va_end(args);
  log_message(INFO_PREFIX, "%s", formatted_message);
}

// Log error messages with [ERROR] prefix
void log_error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char formatted_message[512];
  vsnprintf(formatted_message, sizeof(formatted_message), format, args);
  va_end(args);
  log_message(ERROR_PREFIX, "%s", formatted_message);
}

// Display help menu in the menu window
void print_menu_help() {
  pthread_mutex_lock(&ui->ui_mutex);
  
  werase(ui->menu_window);
  mvwprintw(ui->menu_window, 0, 0, "Available commands:");
  mvwprintw(ui->menu_window, 1, 0, "send -t <destination> -m '<message>' - send message");
  mvwprintw(ui->menu_window, 2, 0, "status - show current router status");
  mvwprintw(ui->menu_window, 3, 0, "exit - exit the program");
  mvwprintw(ui->menu_window, 4, 0, "Command: ");
  wrefresh(ui->menu_window);

  pthread_mutex_unlock(&ui->ui_mutex);
}

// Get user input from the menu window
int get_user_input(char* input, int max_len) {
  // Position cursor at the end of "Command: " line
  wmove(ui->menu_window, 4, 9);
  wrefresh(ui->menu_window);
  
  // Enable echo for user input
  echo();
  int result = wgetnstr(ui->menu_window, input, max_len - 1);
  noecho();
  
  // Clear the input line after reading
  wmove(ui->menu_window, 4, 9);
  wclrtoeol(ui->menu_window);
  wrefresh(ui->menu_window);
  
  return result;
}

// Clean up ncurses interface and free resources
void cleanup_ncurses() {
  // Destroy windows
  delwin(ui->log_window);
  delwin(ui->menu_window);
  
  // Clean up synchronization primitives
  pthread_mutex_destroy(&ui->ui_mutex);
  
  // Free UI structure and reset pointer
  free(ui);
  ui = NULL;
  
  // End ncurses session
  endwin();
}
