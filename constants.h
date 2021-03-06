#ifndef CONSTANTS_H
#define CONSTANTS_H

#define INIT_CMD_BUFFER 999
#define INIT_LINE_BUFFER 32 
#define LINE_BUFFER_INCREMENT 32
#define MAX_MEMORY_ADDR 999
#define MAX_CMD_LENGTH 4
#define MAX_INT 4095
#define MAX_STR_LEN 100

#define IMMEDIATE_ADDR 0
#define DIRECT_ADDR 1
#define INDEX_ADDR 2
#define REGISTER_ADDR 3

#define CODE_TYPE_EXTERNAL 1
#define CODE_TYPE_RELOCATABLE 2

#define GROUP_OFFSET 10
#define OPCODE_OFFSET 6
#define FIRST_ARG_OFFSET 4
#define SECOND_ARG_OFFSET 2
#define BASIC_CMD 28672 /* 111 on the left side */
#define WORD_MASK 0x7FFF

#define CONSTANT_OFFSET 2
#define FIRST_REGISTER_OFFSET 8
#define SECOND_REGISTER_OFFSET 2
#define LABEL_ADDR_OFFSET 2

#define LINE_COMMAND 1
#define LINE_INSTRUCTION 2

#define LABEL_IS_EXTERNAL 1
#define LABEL_IS_ENTRY  2

#define FIRST_ADDR_OFFSET 100
#define SRC_EXT ".as"
#define OBJ_EXT ".ob"
#define EXT_EXT ".ext"
#define ENT_EXT ".ent"
#define WHITESPACE " \t\n"

typedef unsigned short int t_word;


typedef struct label {
    char* name;
    int type;
    int addr;
} t_label;

typedef struct label_proxy {
    char *name;
    char type;
    void *target;
    int flags;
    int offset; /* for data or string labels */
    struct label_proxy *next;
} Label;

typedef struct cmd {
    char* name;
    int opcode;
    int arg_group;
    char dest_types;
    char src_types;
} t_cmd;

typedef struct argument {
    char addr_type;
    t_word value;
    Label *label;
} Argument;

typedef struct cmd_node {
    t_cmd* command;
    int position;
    int size;
    struct argument *dest;
    struct argument *src;
    struct cmd_node *next;
} Command;

#endif
