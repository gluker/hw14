#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "app_state.h"
#include "constants.h"
#include "data_store.h"
#include "commands.h"

char* trim_left(char *line) {
    while (isspace(*line))
        line++;
    return line;
}

char* cut_word(char *line) {
    char *ws;
    ws = strpbrk(line, WHITESPACE);
    if (ws)
        *ws = '\0';
    return line;
}

int store_string(char* string, t_word *start) {
    int offset;
    char *begin, *end;
    begin = strchr(string, '"') + 1;
    if (!begin) {
        log_error("Bad argument: %s", string);
        return -1;
    }
    end = strchr(begin, '"');
    if (!end) {
        log_error("Bad argument: %s", string);
        return -1;
    }
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
    while((arg = strchr(arg, ','))){
        push_data(atoi(++arg));
    }
    return offset;
}

int handle_instruction_line(char *line, void *target){
    char* c = strpbrk(line, WHITESPACE);
    char* args = NULL;

    if (c) {
        args = c + 1;
        *c = '\0';
    }

    if(!args) {
        log_error("Parameter required\n");
        return -1;
    }

    if (!strcmp(line, ".entry")) {
        return store_entry(cut_word(args));
    }
    if (!strcmp(line, ".string")) {
        return store_string(args, target);
    }
    if (!strcmp(line, ".extern")) {
        return store_extern(cut_word(args));
    }
    if (!strcmp(line, ".data")) {
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

    *arg = trim_left(*arg);
    nextarg = strpbrk(*arg, ",");
    if (nextarg) {
        *nextarg = '\0';
        nextarg++;
    } 
    
    cut_word(*arg);

    i = get_register_index(*arg);

    if (i != -1) {
        if (!strchr(*arg, '[')) {
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

    if ((**arg) == '#') {
        argument->addr_type = IMMEDIATE_ADDR;
        argument->value = get_const_code(*arg);   
        *arg = nextarg;
        return argument;
    }

    argument->label = get_label_proxy(*arg);
    if (argument->label) {
        argument->addr_type = DIRECT_ADDR;
    }
    *arg = nextarg;
    return argument;
}

Command* add_args(Command *cmd, char *args) {
    switch(cmd->command->arg_group) {
        case 0:
            break;
        case 1:
            cmd->dest = get_argument(&args);
            if (!check_bitmask(cmd->dest->addr_type, cmd->command->dest_types))
                log_error("Wrong argument type for %s\n", cmd->command->name);
            break;
        case 2:
            cmd->src = get_argument(&args);
            if (!check_bitmask(cmd->src->addr_type, cmd->command->src_types))
                log_error("Wrong argument type for %s\n", cmd->command->name);
            if(!args){
                log_error("Not enough arguments\n");
                break;
            }
            cmd->dest = get_argument(&args);
            if (!check_bitmask(cmd->dest->addr_type, cmd->command->dest_types))
                log_error("Wrong argument type for %s\n", cmd->command->name);
            break;
        default:
            log_error("Application state error - unknown arg group\n");
            exit(1);
            break;
    }
    if(args) {
        while (isspace(*(args++)));
        if (*(args - 1) != '\0')
            log_error("Too much arguments for %s", cmd->command->name);
    }
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
    if(!command){
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
    if (white) {
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

    if (label)
        add_label_proxy(label, line_type, offset, target);
}

void read_from_file(FILE *source){
    char line_buffer[MAX_STR_LEN];

    while(fgets(line_buffer, MAX_STR_LEN, source)) {
        current_line_number++;
        parse_line(line_buffer);
    }
}

void print_data(FILE *file, t_word *data, int index) {
    fprintf(file, "%X %X\n", index, *data);
}

char* str_concat(char* str1, char* str2) {
    char* newstr = malloc(strlen(str1) + strlen(str2));
    if (!newstr) {
        log_error("Cannot allocate memory\n");
        exit(1);
    }
    strcpy(newstr, str1);
    strcat(newstr, str2);
    return newstr;
}

void check_labels_print_entries(FILE* ent_file){
    Label *label;
    t_word addr;
    label = get_labels_head();
    while(label){
        if (label->type == -1 && label->flags ^ LABEL_IS_EXTERNAL)
            log_error("Missing target for label '%s'\n", label->name);
        if (label->flags & LABEL_IS_ENTRY) {
            if (!ent_file)
                log_error("Something went wrong. Application state isn't correct\n");
            addr = label->target ? ((Command*)label->target)->position
                : label->offset + get_cmd_counter();
            fprintf(ent_file, "%s %X\n", label->name, addr + FIRST_ADDR_OFFSET);
        }
        label = label->next;
    }
}

void assemble_files(char* basename){
    Command *current_cmd;  
    FILE *obj_out, *ext_out, *ent_out, *temp_ent;
    char *obj_name, *ext_name, *ent_name;
    char c;
    t_word* data;
    int index;

    temp_ent = tmpfile();
    check_labels_print_entries(temp_ent);

    if (state_has_errors()) {
        fclose(temp_ent);
        return;
    }

    if(state_has_enteries()){
        ent_name = str_concat(basename, ENT_EXT);
        ent_out = fopen(ent_name, "w");
        free(ent_name);
        ent_name = NULL;
        rewind(temp_ent);
        while((c = getc(temp_ent)) != EOF)
            putc(c, ent_out);
        fclose(ent_out);
    }
    fclose(temp_ent);
    

    obj_name = str_concat(basename, OBJ_EXT);
    obj_out = fopen(obj_name, "w");
    free(obj_name);
    obj_name = NULL;

    if(state_has_externs()){
        ext_name = str_concat(basename, EXT_EXT);
        ext_out = fopen(ext_name, "w");
        free(ext_name);
        ext_name = NULL;
    }

    current_cmd = get_commands_head();
    log_debug_info("assembling files for %s\n", basename);

    fprintf(obj_out, "%X %X\n", get_cmd_counter(), get_data_count());
    while (current_cmd){
        index = current_cmd->position + FIRST_ADDR_OFFSET;
         
        fprintf(obj_out, "%X %X\n", index++, 
            get_command_code(current_cmd->command,
            current_cmd->src ? current_cmd->src->addr_type : 0,
            current_cmd->dest ? current_cmd->dest->addr_type : 0));
        if (current_cmd->command->arg_group > 1) {
            fprintf(obj_out, "%X %X\n", index++, get_argument_code(current_cmd->src));
            if (current_cmd->src->label 
                && current_cmd->src->label->flags & LABEL_IS_EXTERNAL)
                fprintf(ext_out, "%s %X\n", current_cmd->src->label->name, index-1);
        }
        if (current_cmd->command->arg_group > 0) {
            fprintf(obj_out, "%X %X\n", index++, get_argument_code(current_cmd->dest));
            if (current_cmd->dest->label 
                && current_cmd->dest->label->flags & LABEL_IS_EXTERNAL)
                fprintf(ext_out, "%s %X\n", current_cmd->dest->label->name, index-1);
        }
        current_cmd = current_cmd->next;
    }

    index = get_cmd_counter() + FIRST_ADDR_OFFSET;
    while((data = pop_head_data())) {
        print_data(obj_out, data, index++);
    }
    fclose(obj_out);
    if(state_has_externs()){
        fclose(ext_out);
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
        if (!current_source){
            printf("Can't open file %s\n", src_filename);
            continue;
        }
        printf("Reading %s...\n", argv[i]);
        clear_state();
        read_from_file(current_source);
        if (state_has_errors()){
            printf("Source isn't correct - not creating output files\n");
            continue;
        }
        assemble_files(argv[i]);
    }
    clear_state();

    return 0;
}
