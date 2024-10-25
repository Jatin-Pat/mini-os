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
    
int curr_pid = -1;

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
int load_script_into_memory(char *filename, int pid) {
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
int create_pcb_for_pid(int pid) {
    if (pcb_array[pid]) {
        return 1;
    } 
    struct pcb_struct *curr_pcb = malloc(sizeof(struct pcb_struct));
    curr_pcb->pid = pid;
    curr_pcb->code = process_code_memory[pid];
    curr_pcb->code_offset = 0;

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
* @param pid the pid to push on the queue
*
* @return: 
*   - 0 when ok 
*/
int ready_queue_push(int pid) {
    struct ready_queue_node *curr_node = malloc(sizeof(struct ready_queue_node));
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
* @returns:
*   - 0 when 0k 
*/
int ready_queue_prepend(int pid) {
    struct ready_queue_node *curr_node = malloc(sizeof(struct ready_queue_node));
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

/**
* Peeks the next element of the queue into ppid. Does not modify the queue.
*
* @returns:
*   - 0 when ok
*   - 1 when queue was already empty
*/
int ready_queue_peek(int *ppid) {
    if (ready_queue.size <= 0) {
        return 1;
    }   
    
    struct ready_queue_node *curr_node = ready_queue.head;
    *ppid = curr_node->pid; 
    return 0;
}

int run_scheduler(char *policy) {
    int error_code = 0;
    char *line;
    struct pcb_struct *curr_pcb;

    

    while (ready_queue.size > 0) {
        // currently FCFS, need to add other algo
        // ready_queue_reorder() // no reorder because FCFS

        // peek because PID needs to stay in queue during execution
        // in case of background execution calling exec: needs to be
        // able to conserve process ordering until current process is
        // handled

        if (curr_pid != -1) {
            return error_code;
        }

        error_code = ready_queue_pop(&curr_pid);
        if (error_code) { return error_code; }

        curr_pcb = pcb_array[curr_pid];
        
        // FCFS: run code until done
        while (curr_pcb->code_offset < MAX_LINES_PER_CODE && curr_pcb->code[curr_pcb->code_offset]){
            curr_pcb = pcb_array[curr_pid];
            line = curr_pcb->code[curr_pcb->code_offset];
            curr_pcb->code_offset++;
            error_code = parseInput(line);         
        }
        
        // cleanup pcb when code is done
        free_script_memory_at_index(curr_pid);
        free_pcb_for_pid(curr_pid);
        curr_pid = -1;
    }
    return error_code;
        
}

