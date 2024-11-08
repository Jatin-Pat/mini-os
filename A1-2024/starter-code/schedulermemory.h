#ifndef SCHEDULERMEMORY_H
#define SCHEDULERMEMORY_H

#define MAX_NUM_PROCESSES 5
#define MAX_LINES_PER_CODE 200

int find_free_pid(int *ppid);
int create_pcb_for_pid(int pid, int line_count);
int free_pcb_for_pid(int pid);
int ready_queue_push(int pid);
int ready_queue_prepend(int pid);
int ready_queue_pop(int *ppid);
int ready_queue_peek(int *ppid);
int get_ready_queue_size();
int run_scheduler();
void ready_queue_reorder_sjf();
void ready_queue_reorder_aging();
int sequential_policy();
int round_robin_policy(int max_timer);
int aging_policy();
#endif
