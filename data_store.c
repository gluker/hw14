#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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



t_label labels_store[100];
int labels_count = 0;
void save_label(char* label, int type, int addr) {
/* if label is cmd, addr is exact. if data - offset from end of cmd part */
    t_label lab;
    lab.name = label;
    lab.type = type; 
    lab.addr = addr;
    labels_store[labels_count++] = lab;
}

t_word get_label_addr(char* label) {
    int i;
    for (i=0; i<labels_count; i++) {
        if (strcmp(labels_store[i].name, label) == 0)
            return labels_store[i].addr;
    }
    return 0;
}


t_word *commands_stack = NULL;
t_word *commands_stack_tail = NULL;
int tail_index = 100;

int push_command(t_word cmd) {
    if (commands_stack == NULL){
        commands_stack = malloc(INIT_CMD_BUFFER * sizeof(t_word));
        commands_stack_tail = commands_stack;
    }
    *(commands_stack_tail++) = cmd;
    return tail_index++;
}


