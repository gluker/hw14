#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "app_state.h"
#include "commands.h"
#include "data_store.h"

#define APP_STATE_ERRORS 1
#define APP_STATE_ENTERIES 2
#define APP_STATE_EXTERNS 4

#define DEBUG

int app_state;
int state_has_errors() {
    return app_state & APP_STATE_ERRORS;
}

int state_has_externs(){
    return app_state & APP_STATE_EXTERNS;
}

int state_has_enteries(){
    return app_state & APP_STATE_ENTERIES;
}

void state_set_externs(){
    app_state |= APP_STATE_EXTERNS;
}
void state_set_enteries(){
    app_state |= APP_STATE_ENTERIES;
}
void clear_state() {
    current_line_number = 0;
    app_state = 0;
    data_store_cleanup();
    commands_cleanup();
}

void log_debug_info(char *fmt, ...) {
    va_list args;

#ifndef DEBUG
    return;
#endif

    va_start(args, fmt);
    printf("%s:%d debug: ", src_filename, current_line_number);
    vprintf(fmt, args);
    va_end(args);
}

void log_warning(char *fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "%s:%d warning: ", src_filename, current_line_number);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

void log_error(char *fmt, ...) {
    
    va_list args;
    app_state |= APP_STATE_ERRORS;
    va_start(args, fmt);
    fprintf(stderr, "%s:%d error: ", src_filename, current_line_number);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void not_implemented() {
    log_debug_info("Not implemented yet\n");
}
