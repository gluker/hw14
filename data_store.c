#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "data_store.h"
#include "constants.h"
#include "app_state.h"

t_word data_store[100];

int data_count = 0;

int get_data_count(){
    return data_count;
}

t_word* pop_head_data(){
    static int head_position;
    if (head_position >= data_count)
        return NULL;
    return &(data_store[head_position++]);
}

t_word* push_data(t_word data) {
    data_store[data_count] = (data & WORD_MASK);
    return &(data_store[data_count++]);
}

int store_entry(char* name){
    Label* label;
    state_set_enteries();
    label = get_label_proxy(name);
    label->flags = LABEL_IS_ENTRY;
    return 0;
}

int store_extern(char *name) {
    Label* label;
    state_set_externs();
    label = add_label_proxy(name, -1, -1, NULL);
    if(!label)
        return -1;
    label->flags = LABEL_IS_EXTERNAL;
    return 0;
}

Label* label_proxies_stack = NULL;
Label* label_proxies_tail = NULL;
Label* get_labels_head() {
    return label_proxies_stack;
}

Label* search_for_proxy(char* name) {
    Label* label;
    label = label_proxies_stack;
    while (label) {
        if (!strcmp(label->name, name))
            return label;
        label = label->next;
    }
    return NULL;
}
int is_valid_label(char* name) {
    if(!isalpha(*name))
        return 0;
    while(*name)
        if(!isalnum(*(name++)))
            return 0;
    return 1;
}

Label* add_label_proxy(char* name, int type, int offset, void *target){
    Label* label;
    if (!is_valid_label(name)) {
        log_error("Invalid label name: '%s'\n", name);
        return NULL;
    }
    label = search_for_proxy(name);
    if (!label){
        label = malloc(sizeof(Label));
        if (!label) {
            log_error("Cannot allocate memory\n");
            exit(1);
        }
        label->name = malloc(strlen(name));
        if (!label->name) {
            log_error("Cannot allocate memory\n");
            exit(1);
        }
        label->next = NULL;
        strcpy(label->name, name);
        if (!label_proxies_stack) {
            label_proxies_stack = label;
        } else {
            label_proxies_tail->next = label;
        }
        label_proxies_tail = label;
    }
    if (label->flags & LABEL_IS_EXTERNAL || label->type > 0 )
        log_error("Label'%s' is already defined\n", name);
    label->type = type;
    label->target = target;
    label->offset = offset;

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
    Label *cur_label, *next_label;
    next_label = label_proxies_stack;
    while (next_label){
        cur_label = next_label;
        next_label = next_label->next;
        free(cur_label->name);
        free(cur_label);
    }
    label_proxies_stack = NULL;
    label_proxies_tail = NULL;

    data_count = 0;
}
