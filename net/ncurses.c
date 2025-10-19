#include "ncurses.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdarg.h>

// Global UI instance
NCursesUI* ui = NULL;

int init_ncurses() {
    // Set locale for Unicode support
    setlocale(LC_ALL, "");
    
    // Initialize ncurses with wide character support
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    refresh();
    
    // Allocate UI structure
    ui = malloc(sizeof(NCursesUI));
    if (!ui) {
        endwin();
        return -1;
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&ui->log_mutex, NULL) != 0) {
        free(ui);
        endwin();
        return -1;
    }
    
    // Initialize log buffer
    ui->log_count = 0;
    ui->log_start = 0;
    ui->initialized = 1;
    
    // Calculate window heights: log window gets 2/3, menu window gets 1/3
    ui->log_window_height = (LINES - 2) * 2 / 3;  // 2/3 of available height (minus borders)
    ui->menu_window_height = (LINES - 2) - ui->log_window_height;  // Remaining 1/3
    
    // Create log window (top portion) with border
    ui->log_window = newwin(ui->log_window_height - 2, COLS - 2, 1, 1);
    if (!ui->log_window) {
        pthread_mutex_destroy(&ui->log_mutex);
        free(ui);
        endwin();
        return -1;
    }
    
    // Create menu window (bottom portion) with border
    ui->menu_window = newwin(ui->menu_window_height - 2, COLS - 2, ui->log_window_height + 1, 1);
    if (!ui->menu_window) {
        delwin(ui->log_window);
        pthread_mutex_destroy(&ui->log_mutex);
        free(ui);
        endwin();
        return -1;
    }
    
    // Draw borders
    box(stdscr, 0, 0);
    mvhline(ui->log_window_height, 0, ACS_HLINE, COLS);
    mvprintw(0, 2, " LOGS ");
    mvprintw(ui->log_window_height, 2, " MENU ");
    refresh();
    
    // Set up windows
    scrollok(ui->log_window, TRUE);
    wrefresh(ui->log_window);
    wrefresh(ui->menu_window);
    
    return 0;
}

void cleanup_ncurses() {
    if (ui) {
        if (ui->log_window) {
            delwin(ui->log_window);
        }
        if (ui->menu_window) {
            delwin(ui->menu_window);
        }
        pthread_mutex_destroy(&ui->log_mutex);
        free(ui);
        ui = NULL;
    }
    endwin();
}

void log_message(const char* prefix, const char* format, ...) {
    if (!ui || !ui->initialized) {
        return;
    }
    
    char formatted_message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(formatted_message, sizeof(formatted_message), format, args);
    va_end(args);
    
    pthread_mutex_lock(&ui->log_mutex);
    
    // Add to circular buffer
    snprintf(ui->log_buffer[ui->log_count], sizeof(ui->log_buffer[ui->log_count]), 
             "%s %s", prefix, formatted_message);
    
    ui->log_count = (ui->log_count + 1) % LOG_BUFFER_SIZE;
    if (ui->log_count == ui->log_start) {
        ui->log_start = (ui->log_start + 1) % LOG_BUFFER_SIZE;
    }
    
    // Clear and redraw log window
    werase(ui->log_window);
    
    // Calculate how many entries we have
    int total_entries = 0;
    if (ui->log_count >= ui->log_start) {
        total_entries = ui->log_count - ui->log_start;
    } else {
        total_entries = LOG_BUFFER_SIZE - ui->log_start + ui->log_count;
    }
    
    // Display the most recent entries that fit in the window
    int max_display = ui->log_window_height - 2; // Account for border
    int start_display = (total_entries > max_display) ? (total_entries - max_display) : 0;
    int y = 0;
    
    for (int i = start_display; i < total_entries && y < max_display; i++) {
        int idx = (ui->log_start + i) % LOG_BUFFER_SIZE;
        mvwprintw(ui->log_window, y, 0, "%s", ui->log_buffer[idx]);
        y++;
    }
    
    wrefresh(ui->log_window);
    pthread_mutex_unlock(&ui->log_mutex);
}

void log_info(const char* format, ...) {
    if (!ui || !ui->initialized) {
        return;
    }
    
    char formatted_message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(formatted_message, sizeof(formatted_message), format, args);
    va_end(args);
    
    log_message(INFO_PREFIX, formatted_message);
}

void log_error(const char* format, ...) {
    if (!ui || !ui->initialized) {
        return;
    }
    
    char formatted_message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(formatted_message, sizeof(formatted_message), format, args);
    va_end(args);
    
    log_message(ERROR_PREFIX, formatted_message);
}

void refresh_menu_window() {
    if (!ui || !ui->menu_window) {
        return;
    }
    wrefresh(ui->menu_window);
}

void refresh_log_window() {
    if (!ui || !ui->log_window) {
        return;
    }
    wrefresh(ui->log_window);
}

void print_menu_help() {
    if (!ui || !ui->menu_window) {
        return;
    }
    
    pthread_mutex_lock(&ui->log_mutex);
    
    werase(ui->menu_window);
    mvwprintw(ui->menu_window, 0, 0, "Available commands:");
    mvwprintw(ui->menu_window, 1, 0, "send -t <destination> -m '<message>' - send message");
    mvwprintw(ui->menu_window, 2, 0, "exit - exit the program");
    mvwprintw(ui->menu_window, 3, 0, "Command: ");
    wrefresh(ui->menu_window);

    pthread_mutex_unlock(&ui->log_mutex);
}

int get_user_input(char* input, int max_len) {
    if (!ui || !ui->menu_window) {
        return 0;
    }
    
    // Position cursor at the end of "Command: " line
    wmove(ui->menu_window, 3, 9);
    wrefresh(ui->menu_window);
    
    // Get input
    echo();
    int result = wgetnstr(ui->menu_window, input, max_len - 1);
    noecho();
    
    // Clear the input line
    wmove(ui->menu_window, 3, 9);
    wclrtoeol(ui->menu_window);
    wrefresh(ui->menu_window);
    
    return result;
}
