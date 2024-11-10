#ifndef SCHEDULERMEMORY_H
#define SCHEDULERMEMORY_H

#define MAX_NUM_PROCESSES 5
#define MAX_LINES_PER_CODE 200
#define MAX_PAGE_TABLE_ENTRIES 20
#define PAGE_SIZE 3 

int process_code_mem_init();
int process_code_mem_deinit();
int free_script_memory();
int create_page_table_for_pid(int pid, char *backing_store_fname);
int load_script_into_memory(char *filename, int pid, int *line_count);
int load_current_script_into_memory(int pid);
#endif
