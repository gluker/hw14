#ifndef COMMANDS_H
#define COMMANDS_H
#include "constants.h"

int get_register_index(char*);
t_word get_const_code(char*);
t_word get_register_code(char*);
t_word get_index_code(char*);
t_word get_label_code(char*);
t_word get_command_code(t_cmd, int, int);


int is_register(char*);
t_cmd get_command(char*);

#endif
