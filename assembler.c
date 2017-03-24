#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "app_state.h"
#include "constants.h"
#include "data_store.h"
#include "commands.h"

char* trim_left(char *line) {
    while (*line == ' ' || *line == '\t')
        line++;
    return line;
}

char* cut_word(char *line) {
    char *ws;
    ws = strpbrk(line, WHITESPACE);
    if (ws != NULL)
        *ws = '\0';
    return line;
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
    push_data('\0');
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

    if (strcmp(line, ".entry") == 0) {
        not_implemented();
        return -1;
    }
    if (strcmp(line, ".string") == 0) {
        return store_string(args, target);
    }
    if (strcmp(line, ".extern") == 0) {
        return store_extern(cut_word(args));
    }
    if (strcmp(line, ".data") == 0) {
        return store_data(args, target);
    }

    log_error("Unrecognized instruction'%s'\n", line);
    return -1;
}

Argument* get_argument(char **arg){
    int i;
    char *nextarg;
    Argument* argument;
    argument = create_argument();

    nextarg = strpbrk(*arg, ",");
    if (nextarg != NULL) {
        *nextarg = '\0';
        nextarg++;
    } 
    
    *arg = trim_left(*arg);
    cut_word(*arg);

    i = get_register_index(*arg);

    if ((*arg)[0] == '#') {
        argument->addr_type = IMMEDIATE_ADDR;
        argument->value = get_const_code(*arg);   
        *arg = nextarg;
        return argument;
    }
    
    if (i != -1) {
        if (strchr(*arg, '[') == NULL) {
            argument->addr_type = REGISTER_ADDR;
            argument->value = get_register_code(*arg);
            *arg = nextarg;
            return argument;
        }
        argument->addr_type = INDEX_ADDR;
        argument->value = get_index_code(*arg);
        *arg = nextarg;
        return argument;
    }

    argument->addr_type = DIRECT_ADDR;
    argument->label = get_label_proxy(*arg);
    *arg = nextarg;
    return argument;
}

Command* add_args(Command *cmd, char *args) {
    switch(cmd->command->arg_group) {
        case 0:
            break;
        case 1:
            cmd->dest = get_argument(&args);
            break;
        case 2:
            cmd->src = get_argument(&args);
            cmd->dest = get_argument(&args);
            break;
        default:
            /*TODO: handle as an error */
            break;
    }
    /*TODO: args should be null */
    return cmd;
}

Command* handle_cmd_line(char *line) {
    char* c = strpbrk(line, WHITESPACE);
    t_cmd* command;
    Command* command_node;
    char* args = NULL;

    if (c != NULL) {
        *c = '\0';
        args = c + 1;
    }

    command = get_command(line);
    if(command == NULL){
        log_error("Unrecognized command '%s'\n", line);
        return NULL;
    }
    command_node = create_command_node(command);
    add_args(command_node, args);

    log_debug_info("Thats a command %s line!\n", command->name);
    return command_node;
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
    log_debug_info("parsing %s\n", line);
    switch(line[0]) {
        case '\n':
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
    char line_buffer[MAX_STR_LEN];

    while(fgets(line_buffer, MAX_STR_LEN, source) != NULL) {
        current_line_number++;
        parse_line(line_buffer);
    }
}

void print_data(FILE *file, t_word *data, int index) {
    fprintf(file, "%X %X\n", index, *data);
}

char* str_concat(char* str1, char* str2) {
    char* newstr = malloc(strlen(str1) + strlen(str2));
    strcpy(newstr, str1);
    strcat(newstr, str2);
    return newstr;
}

void assemble_files(char* basename){
    Command *current_cmd;  
    FILE *obj_out, *ext_out, *ent_out;
    char *obj_name, *ext_name, *ent_name;
    t_word* data;
    int index, cmd_addr;

    obj_name = str_concat(basename, OBJ_EXT);
    ext_name = str_concat(basename, EXT_EXT);
    ent_name = str_concat(basename, ENT_EXT);
    obj_out = fopen(obj_name, "w");
    ext_out = fopen(ext_name, "w");
    ent_out = fopen(ent_name, "w");

    current_cmd = get_commands_head();
    log_debug_info("assembling files for %s\n", basename);

    fprintf(obj_out, "%X %X\n", get_cmd_counter() ,get_data_count());
    while (current_cmd != NULL){
        index = current_cmd->position + FIRST_ADDR_OFFSET;
         
        fprintf(obj_out, "%X %X\n", index++, 
            get_command_code(current_cmd->command,
            current_cmd->src ? current_cmd->src->addr_type : 0,
            current_cmd->dest ? current_cmd->dest->addr_type : 0));
        if (current_cmd->command->arg_group > 1) {
            fprintf(obj_out, "%X %X\n", index++, get_argument_code(current_cmd->src));
            if (current_cmd->src->label != NULL 
                && current_cmd->src->label->flags & LABEL_IS_EXTERNAL)
                fprintf(ext_out, "%s %X\n", current_cmd->src->label->name, index-1);

        }
        if (current_cmd->command->arg_group > 0) {
            fprintf(obj_out, "%X %X\n", index++, get_argument_code(current_cmd->dest));
            if (current_cmd->dest->label != NULL 
                && current_cmd->dest->label->flags & LABEL_IS_EXTERNAL)
                fprintf(ext_out, "%s %X\n", current_cmd->dest->label->name, index-1);
        }
        current_cmd = current_cmd->next;
    }

    index = get_cmd_counter() + FIRST_ADDR_OFFSET;
    while((data = pop_head_data()) != NULL) {
        print_data(obj_out, data, index++);
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
        src_filename = str_concat(argv[i], SRC_EXT);
        current_source = fopen(src_filename, "r");
        if (current_source == NULL){
            printf("Can't open file %s\n", src_filename);
            continue;
        }
        free(src_filename);
        printf("compiling %s...\n", argv[i]);
        clear_state();
        read_from_file(current_source);
        if (!is_source_correct){
            printf("Source isn't correct - not creating output files\n");
            continue;
        }
        assemble_files(argv[i]);
    }

    return 0;
}
