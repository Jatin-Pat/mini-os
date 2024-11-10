#ifndef CODEMEMORY_H 
#define CODEMEMORY_H

#define MAX_NUM_PROCESSES 5
#define MAX_LINES_PER_CODE 30 
#define MAX_PAGE_TABLE_ENTRIES 10 
#define PAGE_SIZE 3 

int process_code_mem_init();
int process_code_mem_deinit();
int free_script_memory();
int free_page_table_for_pid(int pid);
int get_memory_at(int pid, int codeline, char **line);
int create_page_table_for_pid(int pid, char *backing_store_fname);
int load_script_into_memory(int pid, int *line_count);
int load_current_script_into_memory(int pid);
#endif
