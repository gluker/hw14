#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "commands.h"
#include "constants.h"
#include "app_state.h"

int is_register(char* arg) {
    return arg[0] == 'r' && (arg[1] >= '0' || arg[1] <= '8');
}

int get_register_index(char* line) {
    if (!is_register(line))
        return -1;
    return atoi(line + 1);
}

t_word get_register_code(char* line) {
    return atoi(line + 1) << FIRST_REGISTER_OFFSET;
}

t_word get_index_code(char* line) {
    char r1,r2;
    r2 = atoi(line + 1);
    r1 = atoi(strchr(line, '[') + 2);
    return  (r1 << FIRST_REGISTER_OFFSET) |
            (r2 << SECOND_REGISTER_OFFSET);
}

t_word get_argument_code(Argument *arg) {
    switch (arg->addr_type) {
        case IMMEDIATE_ADDR:
        case INDEX_ADDR:
        case REGISTER_ADDR:
            return arg->value;
        case DIRECT_ADDR:
            return get_label_code(arg->label);
    }
    return 0;
}
t_word get_command_code(t_cmd* cmd, int arg1_type, int arg2_type) {
    return BASIC_CMD |
            (cmd->arg_group << GROUP_OFFSET) |
            (cmd->opcode << OPCODE_OFFSET)   |
            (arg1_type << FIRST_ARG_OFFSET) |
            (arg2_type << SECOND_ARG_OFFSET);
}

t_word get_const_code(char* arg) {
    char* temp_arg;
    arg++;
    temp_arg = arg;
    if (*arg == '-' || *arg == '+')
        arg++;
    if (isdigit(*arg)) {
        while(isdigit(*arg)) arg++;
        while(isspace(*arg)) arg++;
        if (*arg == '\0')
        return (atoi(temp_arg) << CONSTANT_OFFSET) & WORD_MASK;
    }
    log_error("Incorrect constant\n");
    return 0;
}

Command* cmd_stack;
Command* cmd_stack_tail;
int cmd_stack_counter = 0;

int get_cmd_counter() {
    return cmd_stack_counter;
}
Command* get_commands_head() {
    return cmd_stack;
}

Command* command_stack_push(Command* cmd) {
    if(!cmd_stack_tail) {
        cmd_stack = cmd;
        cmd_stack_tail = cmd;
        return cmd;
    }
    cmd_stack_tail->next = cmd;
    cmd_stack_tail = cmd;
    return cmd;
}

Command* create_command_node(t_cmd *command) {
    Command* cmd;
    cmd = malloc(sizeof(Command));
    if (!cmd) {
       log_error("Cannot allocate memory\n");
        exit(1);
    }
    cmd->command = command;
    cmd->position = cmd_stack_counter;
    cmd_stack_counter += command->arg_group+1;
    cmd->src = NULL;
    cmd->dest = NULL;
    cmd->next = NULL;
    
    return command_stack_push(cmd);
}

t_word get_label_code(Label* label) {
    if (label->flags & LABEL_IS_EXTERNAL)
        return CODE_TYPE_EXTERNAL;
    if (label->type == LINE_COMMAND)
        return ((((Command*)label->target)->position + FIRST_ADDR_OFFSET ) 
                    << LABEL_ADDR_OFFSET | label->type);

    return ((label->offset + cmd_stack_counter + FIRST_ADDR_OFFSET ) 
                << LABEL_ADDR_OFFSET | label->type);
}

Argument* create_argument() {
    Argument *arg;
    arg = malloc(sizeof(Argument)); 
    if(!arg) {
        log_error("Cannot alocate memory");
        exit(1);
    }
    arg->label = NULL;
    arg->value = 0;
    arg->addr_type = 0;
    return arg;
}

int check_bitmask(char index, char mask) {
    return (1 << index) & mask;
}

int cmd_count = 16;
t_cmd commands[] = {
    {"mov", 0, 2, 
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR) ,
        1 << (IMMEDIATE_ADDR) |
        1 << (DIRECT_ADDR)    |
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  
    }, {"cmp", 1, 2, 
    
        1 << (IMMEDIATE_ADDR) |
        1 << (DIRECT_ADDR)    |
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        1 << (IMMEDIATE_ADDR) |
        1 << (DIRECT_ADDR)    |
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  
    },
    {"add", 2, 2,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR) ,
        1 << (IMMEDIATE_ADDR) |
        1 << (DIRECT_ADDR)    |
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  
    }, {"sub", 3, 2,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR) ,
        1 << (IMMEDIATE_ADDR) |
        1 << (DIRECT_ADDR)    |
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  
    }, {"not", 4, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    }, {"clr", 5, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    }, {"lea", 6, 2,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)      
    }, {"inc", 7, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    
    },
    {"dec", 8, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    },
    {"jmp", 9, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    },
    {"bne", 10, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    },
    {"red", 11, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    },
    {"prn", 12, 1,
        1 << (IMMEDIATE_ADDR) |
        1 << (DIRECT_ADDR)    |
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    },
    {"jsr", 13, 1,
        1 << (DIRECT_ADDR)    | 
        1 << (INDEX_ADDR)     | 
        1 << (REGISTER_ADDR)  ,
        0
    },
    {"rts", 14, 0, 0, 0},
    {"stop", 15, 0, 0, 0},
};
t_cmd* get_command(char* name){

    int i;
    for (i=0; i < cmd_count; i++) {
        if (!strcmp(name, commands[i].name))
            return &commands[i];
    }
    return NULL;
}

void commands_cleanup() {
    Command *cur_cmd, *next_cmd;

    next_cmd = cmd_stack;;
    while (next_cmd){
        cur_cmd = next_cmd;
        next_cmd= next_cmd->next;
        if(cur_cmd->src)
            free(cur_cmd->src);
        if(cur_cmd->dest)
            free(cur_cmd->dest);
        free(cur_cmd);
    }
    cmd_stack = NULL;
    cmd_stack_tail = NULL;
    cmd_stack_counter = 0;

}
