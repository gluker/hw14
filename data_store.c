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

t_label* labels_store[100];
int labels_count = 0;
void save_label(char* label, int type, int addr) {
/* TODO: check pointers for null */
/* if label is cmd, addr is exact. if data - offset from end of cmd part */
    t_label *lab = (t_label*)malloc(sizeof(t_label));
    lab->name = malloc(strlen(label)+1);
    strcpy(lab->name, label);
    lab->type = type; 
    lab->addr = addr;
    labels_store[labels_count++] = lab;
}

t_label *get_label(char* label) {
    int i;
    for (i=0; i<labels_count; i++) {
        if (strcmp(labels_store[i]->name, label) == 0)
            return labels_store[i];
    }
    return NULL;
}

t_word get_label_addr(char* label) {
    int i;
    for (i=0; i<labels_count; i++) {
        if (strcmp(labels_store[i]->name, label) == 0)
            return labels_store[i]->addr;
    }
    return 0;
}

t_word data_store[100];

int data_count = 0;

int get_data_count(){
    return data_count;
}

t_word* push_data(t_word data) {
    data_store[data_count] = data;
    return &(data_store[data_count++]);
}

t_word *commands_stack = NULL;
t_word *commands_stack_tail = NULL;
int tail_index = 100;

int push_command(t_word cmd) {
    if (commands_stack == NULL) {
        commands_stack = malloc(INIT_CMD_BUFFER * sizeof(t_word));
        commands_stack_tail = commands_stack;
    }
    *(commands_stack_tail++) = cmd;
    return tail_index++;
}

Label* label_proxies_stack;
Label* label_proxies_tail;
Label* search_for_proxy(char* name) {
    Label* label;
    label = label_proxies_stack;
    while (label != NULL) {
        if (strcmp(label->name, name) == 0)
            return label;
        label = label->next;
    }
    return NULL;
}
Label* add_label_proxy(char* name, int type, int offset, void *target){
    Label* label;
    label = search_for_proxy(name);
    if (label == NULL){
        label = malloc(sizeof(Label));
        label->type = type;
        label->target = target;
        label->offset = offset;
        label->name = malloc(strlen(name));
        strcpy(label->name, name);
        if (label_proxies_stack == NULL) {
            label_proxies_stack = label;
        } else {
            label_proxies_tail->next = label;
        }
        label_proxies_tail = label;
    } else {
        label->type = type;
        label->target = target;
    }
    return label;
}

Label* get_label_proxy(char* name){
    Label* label;
    label = search_for_proxy(name);
    if (label == NULL)
        label = add_label_proxy(name, -1, -1, NULL);
    return label;
}

void data_store_cleanup(){
    /* TODO: MAKE PROPER CLEANUP!!!! */
    label_proxies_stack = NULL;
    label_proxies_tail = NULL;

    commands_stack = NULL;
    commands_stack_tail = NULL;
    data_count = 0;
}
