#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "schedulermemory.h"
#include "shell.h"

char **process_code_memory[MAX_NUM_PROCESSES];

struct pcb_struct {
    int pid;
    char **code;
    int code_offset;
    int job_length_score;
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
    

int process_code_mem_init() {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        process_code_memory[i] = (char **) calloc(MAX_LINES_PER_CODE, sizeof(char*));
    }
    return 0;
}

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
* @return: 0 if success, 1 if out of PIDs
*/
int find_free_pid(int *ppid) {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        if (!pcb_array[i]) {
            *ppid = i;
            return 0;
        }
    }
    return 1;
}

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

int free_pcb_for_pid(int pid) {
    free(pcb_array[pid]);
    pcb_array[pid] = NULL;
    return 0;
}

/**
* Pushes a new ready node with pid onto ready queue
* 
* @return: 
*   - 0 when ok 
*/
int ready_queue_push(int pid) {
    struct ready_queue_node *curr_node = malloc(sizeof(struct ready_queue_node));
    curr_node->pid = pid;
    curr_node->next = NULL;

    if (!ready_queue.head) {
        ready_queue.head = curr_node;
    }

    if (ready_queue.tail) {
        ready_queue.tail->next = curr_node;
    }

    ready_queue.tail = curr_node;
    ready_queue.size++;

    return 0; 
}

/**
* Pops the last element of the queue into ppid
*
* @returns:
*   - 0 when ok
*   - 1 when queue was already empty
*/
int ready_queue_pop(int *ppid) {
    if (ready_queue.size <= 0) {
        return 1;
    }
    
    struct ready_queue_node *curr_node = ready_queue.head;
    *ppid = curr_node->pid;
    ready_queue.head = curr_node->next;
    free(curr_node);
    ready_queue.size--;
    
    if (ready_queue.size <= 0) {
        ready_queue.tail = NULL;
    }
    return 0;
}

int run_scheduler(char *policy) {
    if (strcmp(policy, "FCFS") == 0) {
        return sequential_policy();

    } else if (strcmp(policy, "SJF") == 0) {
        ready_queue_reorder_sjf(policy);
        return sequential_policy();

    } else if (strcmp(policy, "RR") == 0) {
        return round_robin_policy();

    } else if (strcmp(policy, "AGING") == 0) {
        return aging_policy();

    } else {
        return badcommand();
    }

    return 0;
}

int job_length_compare(const void *a, const void *b) {
    return ((int*)a)[1] - ((int*)b)[1];
}

int ready_queue_reorder_sjf() {
    if (ready_queue.size <= 1) {
        return 0;
    }

    int curr_pid;
    int curr_size = 0;
    int jobs_array[ready_queue.size][2];

    while (ready_queue.size > 0) {
        if (ready_queue_pop(&curr_pid)) {
            return 1;
        }

        jobs_array[curr_size][0] = curr_pid;
        jobs_array[curr_size][1] = pcb_array[curr_pid]->job_length_score;
        curr_size++;
    }

    // sort jobs_array by job_length_score in ascending order
    qsort(jobs_array, curr_size, sizeof(jobs_array[0]), job_length_compare);

    // push sorted jobs back into ready_queue
    for (int i = 0; i < curr_size; i++) {
        ready_queue_push(jobs_array[i][0]);
    }

    return 0;
}

int ready_queue_reorder_aging() {
    if (ready_queue.size <= 1) {
        return 0;
    }

    int pid;
    int curr_size = 0;
    int jobs_array[ready_queue.size][2];

    // Pop all processes from the queue and apply aging to all but the first one
    while (ready_queue.size > 0) {
        if (ready_queue_pop(&pid)) {
            return 1; // Error
        }

        // Apply aging for all jobs except the first one
        if (curr_size != 0 && pcb_array[pid]->job_length_score > 0) {
            pcb_array[pid]->job_length_score -= 1;
        }

        jobs_array[curr_size][0] = pid;
        jobs_array[curr_size][1] = pcb_array[pid]->job_length_score;
        curr_size++;
    }

    for (int i = 1; i < curr_size; i++) {
        if (jobs_array[i][1] < jobs_array[0][1]) {
            qsort(jobs_array, curr_size, sizeof(jobs_array[0]), job_length_compare);
            break;
        }
    }
    
    // Push sorted jobs back into ready_queue
    for (int i = 0; i < curr_size; i++) {
        ready_queue_push(jobs_array[i][0]);
    }

    return 0;
}


int sequential_policy() {
    int curr_pid;
    char *line;
    struct pcb_struct *curr_pcb;
    int error_code = 0;

    while (ready_queue.size > 0) {
        if (ready_queue_pop(&curr_pid)) {
            return 1; // error
        }

        curr_pcb = pcb_array[curr_pid];

        // Execute the program code until done
        while (curr_pcb->code_offset < MAX_LINES_PER_CODE && curr_pcb->code[curr_pcb->code_offset]) {
            line = curr_pcb->code[curr_pcb->code_offset];
            error_code = parseInput(line);         
            curr_pcb->code_offset++;
        }

        // Cleanup pcb when code is done
        free_script_memory_at_index(curr_pid);
        free_pcb_for_pid(curr_pid);
    }

    return error_code;
}

int round_robin_policy() {
    int curr_pid;
    char *line;
    struct pcb_struct *curr_pcb;
    int error_code = 0;

    while (ready_queue.size > 0) {
        if (ready_queue_pop(&curr_pid)) {
            return 1; // error
        }
        int timer = 2;

        curr_pcb = pcb_array[curr_pid];

        // Execute the program code until timer runs out
        while (curr_pcb->code_offset < MAX_LINES_PER_CODE && curr_pcb->code[curr_pcb->code_offset] && timer > 0) {
            line = curr_pcb->code[curr_pcb->code_offset];
            error_code = parseInput(line);         
            curr_pcb->code_offset++;
            timer--;
        }

        // Cleanup pcb when no more code left in process and timer is still running
        if (curr_pcb->code_offset >= MAX_LINES_PER_CODE || curr_pcb->code[curr_pcb->code_offset] == NULL) {
            free_script_memory_at_index(curr_pid);
            free_pcb_for_pid(curr_pid);

        } else {
            // Push back into ready queue if still code left
            ready_queue_push(curr_pid);
        }
    }

    return error_code;
}

int aging_policy() {
    int curr_pid;
    char *line;
    struct pcb_struct *curr_pcb;
    int error_code = 0;

    // Initial SJF ordering based on job length scores
    ready_queue_reorder_sjf();

    while (ready_queue.size > 0) {
        curr_pcb = pcb_array[ready_queue.head->pid];

        // Run one instruction (one time slice) for the current job
        if (curr_pcb->code_offset < MAX_LINES_PER_CODE && curr_pcb->code[curr_pcb->code_offset]) {
            line = curr_pcb->code[curr_pcb->code_offset];
            error_code = parseInput(line);
            curr_pcb->code_offset++;
        }

        // Check if the current job has finished
        if (curr_pcb->code_offset >= MAX_LINES_PER_CODE || curr_pcb->code[curr_pcb->code_offset] == NULL) {
            // Job is complete, free the resources
            ready_queue_pop(&curr_pid);
            free_script_memory_at_index(curr_pid);
            free_pcb_for_pid(curr_pid);
        }

        ready_queue_reorder_aging();
    }

    return error_code;
}
