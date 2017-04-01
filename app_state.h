#ifndef APP_STATE
#define APP_STATE
int current_line_number;
char* src_filename;

void clear_state();
void log_debug_info(char *fmt, ...);
void log_error(char *fmt, ...); 
void log_warning(char *fmt, ...); 
void not_implemented();

int state_has_errors();
int state_has_externs();
int state_has_enteries();

void state_set_externs();
void state_set_enteries();



#endif
