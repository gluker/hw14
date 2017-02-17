#ifndef CONSTANTS_H
#define CONSTANTS_H

#define INIT_LINE_BUFFER 32 
#define LINE_BUFFER_INCREMENT 32
#define MAX_MEMORY_ADDR 999

#define SRC_EXT .as
#define WHITESPACE " \t"

typedef unsigned short int t_word;
typedef struct cmd {
    char* name;
    int opcode;
    int arg_group;
} t_cmd;


#endif
