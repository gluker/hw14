#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "constants.h"

int is_register(char* arg) {
    return arg[0] == 'r' && (arg[1] >= '0' || arg[1] <= '8');
}

int get_register_index(char* line) {
    if (!is_register(line))
        return -1;
    return atoi(line + 1);
}

t_word get_register_code(char* line) {
    return 0;
}

t_word get_index_code(char* line) {
    return 3;
}

t_word get_label_code(char* line) {
    return 6;
}

t_word get_command_code(t_cmd cmd, int arg1_type, int arg2_type) {
    return BASIC_CMD |
            (cmd.arg_group << GROUP_OFFSET) |
            (cmd.opcode << OPCODE_OFFSET) |
            (arg1_type << FIRST_ARG_OFFSET) |
            (arg2_type << SECOND_ARG_OFFSET);
}

t_word get_const_code(char* arg) {
    if (arg[0] == '#')
        arg++;
    return (t_word)atoi(arg) << CONSTANT_OFFSET;
}

t_cmd get_command(char* name){
    int cmd_count = 5;
    t_cmd commands[] = {
        {"mov", 0, 2},
        {"cmp", 1, 2},
        {"add", 2, 2},
        {"sub", 3, 2},
        {"not", 4, 1},
        {"clr", 5, 1}
    };

    t_cmd err = {"error", -1, -1};
    int i;
    for (i=0; i < cmd_count; i++) {
        if (strcmp(name, commands[i].name) == 0)
            return commands[i];
    }
    return err;
}
