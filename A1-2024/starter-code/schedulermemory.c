#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"
#include "schedulermemory.h"
#include "setup.h"
#include "shell.h"

char **process_code_memory[MAX_NUM_PROCESSES];

pcb_t *pcb_array[MAX_NUM_PROCESSES] = {NULL};

ready_queue_t ready_queue = {NULL, NULL, 0};

int curr_pid = -1;

/**
* Returns whether a process is currently being run
*/
char is_process_running() {
    // curr_pid is -1 when not running
    return curr_pid != -1;
}

/**
* Indicates that no process is currently run on this thread
*/
void set_process_not_running() {
    curr_pid = -1;
}

/**
* Indicates that a process is currently running on this thread
*/
void set_process_running(int pid) {
    curr_pid = pid;
}

/**
* Initializes the process code memory.
* @return: 
*   - 0
*/
int process_code_mem_init() {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        process_code_memory[i] = (char **) calloc(CODE_MEM_SIZE, sizeof(char*));
    }
    return 0;
}

/**
* Deinitializes the process code memory.
* @return:
*   - 0
*/
int process_code_mem_deinit() {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        free(process_code_memory[i]);
        process_code_memory[i] = NULL;
    }
    return 0;
}

/**
* Finds the first available pid.
* Places the index of that slot in pointer ppid
* @return: 
*   - 0 if success
*   - error code when not ok
*/
int find_free_pid(int *ppid) {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        if (!pcb_array[i]) {
            *ppid = i;
            return 0;
        }
    }
    return badcommandOutOfPIDs();
}

/**
* Points ppcb to the pcb with pid. 
* 
* Returns: 
*  - 0 on success
*  - 1 if no process exist with pid.
*/
int get_pcb_for_pid(int pid, pcb_t **ppcb) {
   *ppcb = pcb_array[pid]; 
    return (ppcb == NULL);
}

/**
* Allocates memory for a new pcb in the PCB array
* 
* @return: 
*   - 0 if success 
*   - 1 if pcb_array already contains pid 
*/
int create_pcb_for_pid(int pid, int line_count) {
    if (pcb_array[pid]) {
        return 1;
    } 
    pcb_t *curr_pcb = malloc(sizeof(pcb_t));
    curr_pcb->pid = pid;
    curr_pcb->code = process_code_memory[pid];
    curr_pcb->code_offset = 0;
    curr_pcb->job_length_score = line_count;

    pcb_array[pid] = curr_pcb;
    return 0;
}

/**
* Frees the memory allocated for a script at an index.
*
* @param index the index of the script to free
* @return:
*   - 0
*/
int free_script_memory_at_index(int index) {
    int error_code = 0;
    char *pline;

    for (int i = 0; i < CODE_MEM_SIZE; i++) {
        pline = process_code_memory[index][i];
        if (pline) {
            free(pline);
            process_code_memory[index][i] = NULL;
        } 
    }    
    return error_code;
}

/**
* Frees the memory allocated for a pcb at a pid.
*
* @param pid the pid of the pcb to free
* @return:
*   - 0
*/
int free_pcb_for_pid(int pid) {
    free(pcb_array[pid]);
    pcb_array[pid] = NULL;
    return 0;
}

/**
* Pushes a new ready node with pid onto ready queue
* 
* @param pid the pid to push on the queue
*
* @return: 
*   - 0 when ok 
*/
int ready_queue_push(int pid) {
    ready_queue_node_t *curr_node = malloc(sizeof(ready_queue_node_t));
    curr_node->pid = pid;
    curr_node->next = NULL;

    // if list is empty, make curr node the new head
    if (!ready_queue.head) {
        ready_queue.head = curr_node;
    }

    // if there was a tail (list not empty), make it point to curr node
    if (ready_queue.tail) {
        ready_queue.tail->next = curr_node;
    }

    ready_queue.tail = curr_node;
    ready_queue.size++;

    return 0; 
}

/**
* Prepends an element to the front of the queue. 
*
* @param pid the pid to place in front of the queue
*
* @return:
*   - 0 when 0k 
*/
int ready_queue_prepend(int pid) {
    ready_queue_node_t *curr_node = malloc(sizeof(ready_queue_node_t));
    curr_node->pid = pid;
    curr_node->next = ready_queue.head;    

    // if list is empty, ensure the tail will point to curr node
    if (!ready_queue.head) {
       ready_queue.tail = curr_node; 
    }
    ready_queue.head = curr_node;
    ready_queue.size++;
    
    return 0;
}

/**
* Pops the next element of the queue into ppid
*
* @return:
*   - 0 when ok
*   - 1 when queue was already empty
*/
int ready_queue_pop(int *ppid) {
    if (get_ready_queue_size() <= 0) {
        return 1;
    }
    
    ready_queue_node_t *curr_node = ready_queue.head;
    *ppid = curr_node->pid;
    ready_queue.head = curr_node->next;
    free(curr_node);
    ready_queue.size--;
    
    if (ready_queue.size <= 0) {
        ready_queue.tail = NULL;
    }
    return 0;
}

/**
* Peeks the next element of the queue into ppid. Does not modify the queue.
*
* @return:
*   - 0 when ok
*   - 1 when queue was already empty
*/
int ready_queue_peek(int *ppid) {
    if (get_ready_queue_size() <= 0) {
        return 1;
    }   
    
    ready_queue_node_t *curr_node = ready_queue.head;

    *ppid = curr_node->pid; 
    return 0;
}

/**
* Gets the size of the ready queue in a thread-safe way.
*
* @returs: the size of the ready queue
*/
int get_ready_queue_size() {
    int size = ready_queue.size;
    return size;
}

/**
* comparison function for qsort
*
* @return:
*   - 0 if a == b
*   - <0 if a < b
*   - >0 if a > b
*/
int job_length_compare(const void *a, const void *b) {
    return ((int*)a)[1] - ((int*)b)[1];
}

/**
* Reorders the ready queue in acsending order of job length score.
*/
void ready_queue_reorder_sjf() {
    if (get_ready_queue_size() <= 1) {
        return;
    }

    int jobs_array[get_ready_queue_size()][2];
    int curr_pid;
    int curr_index = 0;

    // Pop jobs from ready queue and store as pid, job_length_score pair.
    // An array is used to simplify operations with qsort, over directly sorting the ready_queue linked list.
    // We need to store the job pid so that after reordering we know which job the job_length_score belongs to.
    while (get_ready_queue_size() > 0) {
        ready_queue_pop(&curr_pid);
        jobs_array[curr_index][0] = curr_pid;
        jobs_array[curr_index][1] = pcb_array[curr_pid]->job_length_score;
        curr_index++;
    }
    
    // Each element in jobs_array has size equal to a pair of pid, job_length_score by construction.
    qsort(jobs_array, curr_index, sizeof(jobs_array[0]), job_length_compare);

    for (int i = 0; i < curr_index; i++) {
        ready_queue_push(jobs_array[i][0]);
    }
}

/**
* Reorders the ready queue with lowest job length score placed at the head.
*
* @param pid: the pid of the job that just ran.
*/
void ready_queue_reorder_aging(int pid) {
    if (get_ready_queue_size() <= 1) {
        return;
    }

    int jobs_array[get_ready_queue_size()][2];
    int curr_pid;
    int curr_index = 0;

    while (get_ready_queue_size() > 0) {
        ready_queue_pop(&curr_pid);
        
        // Decrement job_length_score for all jobs except the one that just ran .
        if (curr_pid != pid && pcb_array[curr_pid]->job_length_score > 0) {
            pcb_array[curr_pid]->job_length_score--;
        }
        
        jobs_array[curr_index][0] = curr_pid;
        jobs_array[curr_index][1] = pcb_array[curr_pid]->job_length_score;
        curr_index++;
    }
    qsort(jobs_array, curr_index, sizeof(jobs_array[0]), job_length_compare);

    for (int i = 0; i < curr_index; i++) {
        ready_queue_push(jobs_array[i][0]);
    }
}


