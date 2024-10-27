#ifndef SCHEDULERMEMORY_H
#define SCHEDULERMEMORY_H

#define MAX_NUM_PROCESSES 4
#define MAX_LINES_PER_CODE 100

int process_code_mem_init();
int process_code_mem_deinit();
int find_free_pid(int *ppid);
int create_pcb_for_pid(int pid, int line_count);
int load_script_into_memory(char *filename, int pid, int *line_count);
int free_script_memory_at_index(int index);
int free_pcb_for_pid(int pid);
int ready_queue_push(int pid);
int ready_queue_pop(int *ppid);
int run_scheduler();
void ready_queue_reorder_sjf();
void ready_qeueu_reorder_aging();
int sequential_policy();
int round_robin_policy();
int aging_policy();
#endif
