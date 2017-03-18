#ifndef COMMANDS_H
#define COMMANDS_H
#include "constants.h"

void commands_cleanup();

int get_register_index(char*);
t_word get_const_code(char*);
t_word get_register_code(char*);

t_word get_index_code(char*);
t_word get_label_code(char*);
t_word get_command_code(t_cmd*, int, int);

t_word get_argument_code(Argument *arg);

Argument* create_argument();
Command* create_command_node(t_cmd*);
Command* command_stack_push(Command*);
Command* get_cmd_head();

int is_register(char*);
t_cmd* get_command(char*);

#endif
