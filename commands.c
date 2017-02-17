#include "commands.h"
#include "constants.h"

int is_register(char* arg) {
    return arg[0] == 'r' && (arg[1] >= '0' || arg[1] <= '8');
}

t_cmd commands[1] = {{"mov", 0, 2}};

t_cmd get_command(char* name){
    t_cmd mov = {"mov", 0, 2};
    return mov;
}
