#include "constants.h"

#ifndef DATA_STORE_H
#define DATA_STORE_H

void data_store_cleanup();
void push_to_buffer(char**, char);
void save_label(char*, int, int);
t_label* get_label(char*);
t_word get_label_addr(char*);

Label* get_label_proxy(char*);
Label* add_label_proxy(char*, int, int, void*);


int push_command(t_word);
int get_data_count();
t_word* push_data(t_word);

#endif
