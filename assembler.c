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

int handle_instruction_line(char *line){
    char* c = strpbrk(line, WHITESPACE);
    char* args = NULL;
    
    if (c != NULL) {
        args = c + 1;
        *c = '\0';
    }
    
    if (strcmp(line, ".entery") == 0) {
        return;
    }
    if (strcmp(line, ".string") == 0) {
        return;
    }
    if (strcmp(line, ".extern") == 0) {
        return;
    }
    if (strcmp(line, ".data") == 0) {
        return;
    }

    fprintf(stderr, "Unrecognized instruction'%s'\n", line);
}

int handle_cmd_line(char *line) {
    /*  
        imply that line it trimmed
        returns command index in stack
    */
    t_word arg1, arg2, cmd;
    int arg1_type = 0, arg2_type = 0, cmd_index;
    char* c = strpbrk(line, WHITESPACE);
    t_cmd command;
    char* args = NULL;

    if (c != NULL) {
        *c = '\0';
        args = c + 1;
        parse_args(args, &arg1, &arg1_type, &arg2, &arg2_type);
    }

    command = get_command(line);

    switch (command.arg_group) {
        case 0:
            cmd = get_command_code(command, arg1_type, arg2_type);
            cmd_index = push_command(cmd);
            break;
        case 1:
            cmd = get_command_code(command, arg2_type, arg2_type);
            cmd_index = push_command(cmd);
            push_command(arg1);
            break;
        case 2:
            cmd = get_command_code(command, arg1_type, arg2_type);
            cmd_index = push_command(cmd);
            push_command(arg1);
            push_command(arg2);
            break;
        default:
            fprintf(stderr, "Unrecognized command '%s'\n", line);
            return -1;
    }

    printf("Thats a command %s line!\n", command.name);
    return cmd_index;
}


int is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'Z');
}

void parse_line(char *line){
    char *white;
    char *label = NULL;
    int line_type, label_addr;
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
            label_addr = handle_instruction_line(line);
            line_type = LINE_INSTRUCTION;
            break;
        default:
            line_type = LINE_COMMAND;
            label_addr = handle_cmd_line(line);
    }

    if (label != NULL)
        save_label(label, line_type, label_addr);

}

void assemble(FILE *source){
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

int main(int argc, char *argv[])
{
    int i=0; FILE *current_source;

    if (argc < 2) {
        printf("Specify names of .as files to assemble\n");
        return 1;
    }

    for (i=1; i < argc; i++){
        current_source = fopen(argv[i], "r");
        if (current_source == NULL){
            printf("Can't open file %s\n", argv[i]);
            continue;
        }
        printf("compiling %s...\n", argv[i]);
        assemble(current_source);
    }

    return 0;
}
