#ifndef SCHEDULER_MEMORY_H
#define SCHEDULER_MEMORY_H

#define MAX_NUM_PROCESSES 4
#define MAX_LINES_PER_CODE 100

int process_code_mem_init();
int find_free_script_memory_index(int *pindex);
int load_script_into_memory(char *filename, int *ppid);
int free_script_memory_at_index(int index);
#endif
