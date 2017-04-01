#include "constants.h"

#ifndef DATA_STORE_H
#define DATA_STORE_H

void data_store_cleanup();
t_word get_label_addr(char*);

Label* get_label_proxy(char*);
Label* add_label_proxy(char*, int, int, void*);
Label* get_labels_head();

t_word* pop_head_data();
int store_extern(char*);
int store_entry(char*);

int get_data_count();
t_word* push_data(t_word);

#endif
