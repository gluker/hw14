#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data_store.h"
#include "commands.h"

char* trim_left(char *line) {
    while (*line == ' ' || *line == '\t')
        line++;
    return line;
}

t_word parse_argument(char *argument, int* type){
    int i;

    argument = trim_left(argument);
    i = get_register_index(argument);

    if (argument[0] == '#') {
        printf("Immediate addr\n");
        *type = IMMEDIATE_ADDR;
        return get_const_code(argument);   
    }
    
    if (i != -1) {
        if (strchr(argument, '[') == NULL) {
            printf("%s is a register: %d\n", argument, i);
            *type = REGISTER_ADDR;
            return get_register_code(argument);
        }
        printf("%s is an index: %d\n", argument, i);
        *type = INDEX_ADDR;
        return get_index_code(argument);
    }

    *type = DIRECT_ADDR;
    return get_label_code(argument);
}

void parse_args(char *arguments, 
        t_word *arg1, int *arg1_type, t_word *arg2, int *arg2_type){
    char* comma;

    if (arguments == NULL)
        return;

    comma = strchr(arguments, ',');
    if (comma != NULL) {
        *comma = '\0'; 
        *arg2 = parse_argument(comma + 1, arg2_type);
    }

    *arg1 = parse_argument(arguments, arg1_type);
}

int store_string(char* string, t_word *start) {
    int offset;
    char *begin, *end;
    begin = strchr(string, '"') + 1;
    end = strchr(begin, '"');
    offset = get_data_count();
    start = push_data(*(begin++));
    while (begin < end)
        push_data(*(begin++));
    return offset;
}

int store_data(char* arg, t_word *start) {
    int offset;
    offset = get_data_count();
    start = push_data(atoi(arg));
    while((arg = strchr(arg, ',')) != NULL){
        push_data(atoi(++arg));
    }
    return offset;
}

int handle_instruction_line(char *line, void *target){
    char* c = strpbrk(line, WHITESPACE);
    char* args = NULL;
    
    if (c != NULL) {
        args = c + 1;
        *c = '\0';
    }
    
    if (strcmp(line, ".entery") == 0) {
        return -1;
    }
    if (strcmp(line, ".string") == 0) {
        return store_string(args, target);
    }
    if (strcmp(line, ".extern") == 0) {
        return -1;
    }
    if (strcmp(line, ".data") == 0) {
        return store_data(args, target);
    }

    fprintf(stderr, "Unrecognized instruction'%s'\n", line);
    return -1;
}

Argument* get_argument(char *arg){
    int i;
    char *nextarg;
    Argument* argument;
    argument = create_argument();

    nextarg = strchr(arg, ',');
    if (nextarg != NULL) {
        *nextarg = '\n';
        nextarg++;
    } 
    
    arg = trim_left(arg);
    i = get_register_index(arg);

    if (arg[0] == '#') {
        argument->addr_type = IMMEDIATE_ADDR;
        argument->value = get_const_code(arg);   
        arg = nextarg;
        return argument;
    }
    
    if (i != -1) {
        if (strchr(arg, '[') == NULL) {
            argument->addr_type = REGISTER_ADDR;
            argument->value = get_register_code(arg);
            arg = nextarg;
            return argument;
        }
        argument->addr_type = INDEX_ADDR;
        argument->value = get_index_code(arg);
        arg = nextarg;
        return argument;
    }

    argument->addr_type = DIRECT_ADDR;
    argument->label = get_label_proxy(arg);
    arg = nextarg;
    return argument;
}
Command* add_args(Command *cmd, char *args) {
    switch(cmd->command->arg_group) {
        case 0:
            break;
        case 1:
            cmd->dest = get_argument(args);
            break;
        case 2:
            cmd->src = get_argument(args);
            cmd->dest = get_argument(args);
            break;
        default:
            /*TODO: handle as an error */
            break;
    }
    /*TODO: args should be null */
    return cmd;
}

Command* handle_cmd_line(char *line) {
    t_word arg1, arg2;
    int arg1_type = 0, arg2_type = 0;
    char* c = strpbrk(line, WHITESPACE);
    t_cmd* command;
    Command* command_node;
    char* args = NULL;

    if (c != NULL) {
        *c = '\0';
        args = c + 1;
        parse_args(args, &arg1, &arg1_type, &arg2, &arg2_type);
    }

    command = get_command(line);
    if(command == NULL){
        fprintf(stderr, "Unrecognized command '%s'\n", line);
        return NULL;
    }
    command_node = create_command_node(command);
    add_args(command_node, args);

    printf("Thats a command %s line!\n", command->name);
    return command_node;
}


int is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'Z');
}

void parse_line(char *line){
    char *white;
    char *label = NULL;
    int line_type, offset = -1;
    void *target = NULL;
    if (line[0] == ';')
        return;
    white = strchr(line, ':');
    if (white != NULL) {
        label = line;
        line = white + 1;
        *white = '\0';
    }
    
    line = trim_left(line);
    printf("parsing %s\n", line);
    switch(line[0]) {
        case '\0':
            return;
        case '.':
            offset = handle_instruction_line(line, target);
            line_type = LINE_INSTRUCTION;
            break;
        default:
            line_type = LINE_COMMAND;
            target = handle_cmd_line(line);
    }

    if (label != NULL)
        add_label_proxy(label, line_type, offset, target);

}

void read_from_file(FILE *source){
    char *line_buffer = NULL;
    char c;

    while(1) {
        c = fgetc(source);
        switch (c) {
            case EOF:
                return;
            case '\n':
                push_to_buffer(&line_buffer, '\0');
                parse_line(line_buffer);
                break;
            default:
                push_to_buffer(&line_buffer, c);
        }
    }
}

void assemble_files(char* basename){
    Command *current_cmd;  
    
}

char* str_concat(char* str1, char* str2) {
    char* newstr = malloc(strlen(str1) + strlen(str2));
    strcpy(newstr, str1);
    strcat(newstr, str2);
    return newstr;
}

int main(int argc, char *argv[])
{
    int i=0; FILE *current_source;
    char* src_filename;
    if (argc < 2) {
        printf("Specify names of .as files to assemble\n");
        return 1;
    }

    for (i=1; i < argc; i++){
        src_filename = str_concat(argv[i], SRC_EXT);
        current_source = fopen(src_filename, "r");
        if (current_source == NULL){
            printf("Can't open file %s\n", src_filename);
            continue;
        }
        printf("compiling %s...\n", argv[i]);
        read_from_file(current_source);
    }

    free(src_filename);
    return 0;
}
