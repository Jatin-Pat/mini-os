#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"
#include "schedulermemory.h"
#include "shell.h"

char **process_code_memory[MAX_NUM_PROCESSES];

struct pcb_struct {
    int pid;
    char **code;
    int code_offset;
    int job_length_score; // initialized to line_count
};

struct pcb_struct *pcb_array[MAX_NUM_PROCESSES] = {NULL};

struct ready_queue_node {
    int pid;
    struct ready_queue_node *next;
};

struct ready_queue_struct {
    struct ready_queue_node *head;
    struct ready_queue_node *tail;
    int size;
} ready_queue = {NULL, NULL, 0};

pthread_mutex_t ready_queue_lock = PTHREAD_MUTEX_INITIALIZER;
    
__thread int curr_pid = -1;

/**
* Initializes the process code memory.
* @return: 
*   - 0
*/
int process_code_mem_init() {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        process_code_memory[i] = (char **) calloc(MAX_LINES_PER_CODE, sizeof(char*));
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
* Loads the script contained in filename into process memory for a pid.
*
* @param filename the name of the file to load
* @param pid the pid of the process in which to load the file
*
* @return:
*   - 0 when ok
*   - error code when not ok
*/
int load_script_into_memory(char *filename, int pid, int *line_count) {
    char line[MAX_USER_INPUT];
    int error_code = 0;
    int current_line = 0;

    FILE *p = fopen(filename, "rt");
    if (!p) {
        return badcommandFileDoesNotExist();
    }

    // for each line in file, load into memory[free_index]
    fgets(line, MAX_USER_INPUT, p);
    while (1) {
        process_code_memory[pid][current_line] = strdup(line);
        current_line++;

        memset(line, 0, sizeof(line));
        if (feof(p)) {
            break;
        }
        if (!fgets(line, MAX_USER_INPUT, p)) {
            break;
        }
    }

    fclose(p);

    *line_count = current_line;
    return error_code;
}

/**
* Loads the rest of the current script into process memory for a pid.
*
* Requires the shell to be executing in batch mode.
* 
* @param pid the pid in which to load the script.
*
* @returns:
*   - 0 when ok
*   - error code when not ok
*/
int load_current_script_into_memory(int pid) {
    char line[MAX_USER_INPUT];
    int error_code = 0;
    int current_line = 0;
    
    while(1) {
        if (isatty(0)){
            return exceptionCannotLoadInteractiveScript();
        } else if (feof(stdin)) {
            break;
        }

        if (!fgets(line, MAX_USER_INPUT, stdin)) {
            break;
        }

        process_code_memory[pid][current_line] = strdup(line);
        current_line++;
        memset(line, 0, sizeof(line));
    }

    return error_code;
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
    struct pcb_struct *curr_pcb = malloc(sizeof(struct pcb_struct));
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

    for (int i = 0; i < MAX_LINES_PER_CODE; i++) {
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
    struct ready_queue_node *curr_node = malloc(sizeof(struct ready_queue_node));
    curr_node->pid = pid;
    curr_node->next = NULL;

    pthread_mutex_lock(&ready_queue_lock);
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
    pthread_mutex_unlock(&ready_queue_lock);

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
    struct ready_queue_node *curr_node = malloc(sizeof(struct ready_queue_node));
    curr_node->pid = pid;
    curr_node->next = ready_queue.head;    

    pthread_mutex_lock(&ready_queue_lock);
    // if list is empty, ensure the tail will point to curr node
    if (!ready_queue.head) {
       ready_queue.tail = curr_node; 
    }
    ready_queue.head = curr_node;
    ready_queue.size++;
    pthread_mutex_unlock(&ready_queue_lock);
    
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
    
    pthread_mutex_lock(&ready_queue_lock);
    struct ready_queue_node *curr_node = ready_queue.head;
    *ppid = curr_node->pid;
    ready_queue.head = curr_node->next;
    free(curr_node);
    ready_queue.size--;
    
    if (ready_queue.size <= 0) {
        ready_queue.tail = NULL;
    }
    pthread_mutex_unlock(&ready_queue_lock);
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
    
    pthread_mutex_lock(&ready_queue_lock);
    struct ready_queue_node *curr_node = ready_queue.head;
    pthread_mutex_unlock(&ready_queue_lock);

    *ppid = curr_node->pid; 
    return 0;
}

/**
* Gets the size of the ready queue in a thread-safe way.
*
* @returs: the size of the ready queue
*/
int get_ready_queue_size() {
    pthread_mutex_lock(&ready_queue_lock);
    int size = ready_queue.size;
    pthread_mutex_unlock(&ready_queue_lock);
    return size;
}

/**
* Runs the scheduler for a policy.
* 
* @param policy: the policy to run
* @return:
*   - 0 when ok
*   - error code when not ok
*/
int run_scheduler(char *policy) {
    int error_code = 0;
    
    if (curr_pid != -1) {
        return error_code;
    }

    if (strcmp(policy, "FCFS") == 0) {
        error_code = sequential_policy();

    } else if (strcmp(policy, "SJF") == 0) {
        ready_queue_reorder_sjf(policy);
        error_code = sequential_policy();

    } else if (strcmp(policy, "RR") == 0) {
        error_code = round_robin_policy(2);

    } else if (strcmp(policy, "RR30") == 0) {
        error_code = round_robin_policy(30);
    
    } else if (strcmp(policy, "AGING") == 0) {
        error_code = aging_policy();

    } else {
        return badcommandInvalidPolicy();
    }

    curr_pid = -1;
    return error_code;
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

/**
* Sequentially runs each job until completion in the order they were loaded in the ready queue.
*
* If policy is FCFS, the ready queue is already in the correct order.
* If policy is SJF, ready_queue_reorder_sjf() must be called prior to reorder the ready queue in ascending order of job length score.
* @return: 
*   - 0 if success
*   - 1 if error
*/
int sequential_policy() {
    char *line;
    struct pcb_struct *curr_pcb;
    int error_code = 0;

    while (get_ready_queue_size() > 0) {
        error_code = ready_queue_pop(&curr_pid);
        if (error_code) { return error_code; }

        curr_pcb = pcb_array[curr_pid];

        while (curr_pcb->code_offset < MAX_LINES_PER_CODE && curr_pcb->code[curr_pcb->code_offset]) {
            line = curr_pcb->code[curr_pcb->code_offset];
            curr_pcb->code_offset++;
            error_code = parseInput(line);         
        }
        // Job is done, free up resources
        free_script_memory_at_index(curr_pid);
        free_pcb_for_pid(curr_pid);
    }

    return error_code;
}

/**
* Runs each job for a time slice of 2 instructions.
*
* If a job is not completed in the time slice, it is pushed back in the back of the ready queue.
* @return:
*   - 0 if success
*   - 1 if error
*/
int round_robin_policy(int max_timer) {
    char *line;
    struct pcb_struct *curr_pcb;
    int error_code = 0;
    int timer = max_timer;

    while (get_ready_queue_size() > 0) {
        if (ready_queue_pop(&curr_pid)) {
            return 1; // error
        }
        curr_pcb = pcb_array[curr_pid];
        timer = max_timer;

        while (curr_pcb->code_offset < MAX_LINES_PER_CODE && curr_pcb->code[curr_pcb->code_offset] && timer > 0) {
            line = curr_pcb->code[curr_pcb->code_offset];
            usleep(1);
            curr_pcb->code_offset++;
            timer--;
            error_code = parseInput(line);         
        }

        if (curr_pcb->code_offset >= MAX_LINES_PER_CODE || !curr_pcb->code[curr_pcb->code_offset]) {
            free_script_memory_at_index(curr_pid);
            free_pcb_for_pid(curr_pid);

        } else {
            ready_queue_push(curr_pid);
        }
    }

    return error_code;
}

/**
* Runs the head job in the ready queue for a time slice of 1 instruction, aging all other jobs each iteration.
*
* If job with lower job length score is present after ready_queue_reorder_aging(), current process is preempted, 
* and lowest job length score placed at the head for next iteration.
* Jobs are only poped if completed; allowing current job to continue if it has the lowest score after ready_queue_reorder_aging().
* @return:
*   - 0 if success
*   - 1 if error
*/
int aging_policy() {
    char *line;
    struct pcb_struct *curr_pcb;
    int error_code = 0;

    ready_queue_reorder_sjf();

    while (get_ready_queue_size() > 0) {
        curr_pid = ready_queue.head->pid;
        curr_pcb = pcb_array[curr_pid];

        if (curr_pcb->code_offset < MAX_LINES_PER_CODE && curr_pcb->code[curr_pcb->code_offset]) {
            line = curr_pcb->code[curr_pcb->code_offset];
            error_code = parseInput(line);
            curr_pcb->code_offset++;
        }

        if (curr_pcb->code_offset >= MAX_LINES_PER_CODE || !curr_pcb->code[curr_pcb->code_offset]) {
            ready_queue_pop(&curr_pid);
            free_script_memory_at_index(curr_pid);
            free_pcb_for_pid(curr_pid);
        }
        ready_queue_reorder_aging(curr_pid);
    }

    return error_code;
}
