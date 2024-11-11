#ifndef SCHEDULERMEMORY_H
#define SCHEDULERMEMORY_H

typedef struct {
    int pid;
    char **code;
    int code_offset;
    int job_length_score; // initialized to line_count
} pcb_t;

typedef struct ready_queue_node_t {
    int pid;
    struct ready_queue_node_t *next;
} ready_queue_node_t;

typedef struct {
    ready_queue_node_t *head;
    ready_queue_node_t *tail;
    int size;
} ready_queue_t;


char is_process_running();
void set_process_not_running();
void set_process_running(int pid);
int get_pcb_for_pid(int pid, pcb_t **ppcb);
int process_code_mem_init();
int process_code_mem_deinit();
int find_free_pid(int *ppid);
int create_pcb_for_pid(int pid, int line_count);
int load_script_into_memory(char *filename, int pid, int *line_count);
int load_current_script_into_memory(int pid);
int free_script_memory_at_index(int index);
int free_pcb_for_pid(int pid);
int ready_queue_push(int pid);
int ready_queue_prepend(int pid);
int ready_queue_pop(int *ppid);
int ready_queue_peek(int *ppid);
int get_ready_queue_size();
void ready_queue_reorder_sjf();
void ready_queue_reorder_aging();

#endif
