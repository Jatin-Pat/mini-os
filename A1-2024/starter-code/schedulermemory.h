#ifndef SCHEDULERMEMORY_H
#define SCHEDULERMEMORY_H

#define MAX_NUM_PROCESSES 4
#define MAX_LINES_PER_CODE 100

int process_code_mem_init();
int process_code_mem_deinit();
int find_free_pid(int *ppid);
int create_pcb_for_pid(int pid);
int load_script_into_memory(char *filename, int pid);
int load_current_script_into_memory(int pid);
int free_script_memory_at_index(int index);
int free_pcb_for_pid(int pid);
int ready_queue_push(int pid);
int ready_queue_prepend(int pid);
int ready_queue_pop(int *ppid);
int ready_queue_peek(int *ppid);
int run_scheduler();
#endif
