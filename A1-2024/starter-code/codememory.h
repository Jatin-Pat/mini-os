#ifndef CODEMEMORY_H
#define CODEMEMORY_H

#include "setup.h"

typedef struct {
    char *backing_store_fname;
    int *entries;
} page_table_t;

int code_mem_init();
int code_mem_deinit();
int free_script_memory();
int create_page_table_for_pid(int pid, char *backing_store_fname);
int free_page_table_for_pid(int pid);
int load_page_at(int pid, int codeline);
int get_memory_at(int pid, int codeline, char **line);
int handle_page_fault(int pid, int codeline);
int evict_frame(int pid, int codeline);
int load_script_into_memory(int pid, int *line_count);
int load_current_script_into_memory(int pid);

#endif
