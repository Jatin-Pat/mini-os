#ifndef SCHEDULERMEMORY_H
#define SCHEDULERMEMORY_H

#define MAX_NUM_PROCESSES 5
#define MAX_LINES_PER_CODE 200

int process_code_mem_init();
int process_code_mem_deinit();
int load_script_into_memory(char *filename, int pid, int *line_count);
int load_current_script_into_memory(int pid);
int free_script_memory(int index);
#endif
