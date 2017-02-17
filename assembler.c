#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data_store.h"

char parse_argument(char *argument, int N){
    return 'x';
}


void handle_instruction_line(char *line){

}


int is_alpha(char c) {
    return (c > 'A' && c < 'Z') || (c > 'a' && c < 'Z');
}

void parse_line(char *line){
    char *white;
    if (line[0] == ';')
        return;
    if (is_alpha(line[0])) {
        white = strpbrk(line, " \t:");
        if (*white == ':'){
           *white = '\0';
           save_label(line);
           parse_line(white + 1);
           return;
        }
    }
    
    while (*line == ' ' || *line == '\t')
        line++;
    printf("parsing %s\n", line);
    if (line[0] == '.')
        handle_instruction_line(line);

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
