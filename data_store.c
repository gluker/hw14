#include <stdio.h>
#include <stdlib.h>

#include "data_store.h"
#include "constants.h"

void push_to_buffer(char **buffer, char c){
    static int is_allocated = 0;
    static int current_length = 0;
    static int current_max_length = INIT_LINE_BUFFER;
    if (!is_allocated) {
        *buffer = (char*) malloc(INIT_LINE_BUFFER);
        is_allocated = 1;
    }
    if (current_length == current_max_length){
        current_max_length += LINE_BUFFER_INCREMENT;
        *buffer = (char*) realloc(*buffer, current_max_length);
    }
    *(*buffer + current_length++) = c;
    if (c == '\0')
        current_length = 0;
}

void save_label(char* label) {
    printf("saving label %s\n", label);
}

t_word get_label_addr(char* label) {
    return 0;
}
