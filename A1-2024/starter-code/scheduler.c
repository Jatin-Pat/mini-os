#include "schedulermemory.h"

#include "scheduler.h"

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
    pcb_t *curr_pcb;
    int error_code = 0;

    while (get_ready_queue_size() > 0) {
        error_code = ready_queue_pop(&curr_pid);
        if (error_code) { return error_code; }

        curr_pcb = pcb_array[curr_pid];

        while (curr_pcb->code_offset < CODE_MEM_SIZE && curr_pcb->code[curr_pcb->code_offset]) {
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
    pcb_t *curr_pcb;
    int error_code = 0;
    int timer = max_timer;

    while (get_ready_queue_size() > 0) {
        if (ready_queue_pop(&curr_pid)) {
            return 1; // error
        }
        curr_pcb = pcb_array[curr_pid];
        timer = max_timer;

        while (curr_pcb->code_offset < CODE_MEM_SIZE && curr_pcb->code[curr_pcb->code_offset] && timer > 0) {
            line = curr_pcb->code[curr_pcb->code_offset];
            usleep(1);
            curr_pcb->code_offset++;
            timer--;
            error_code = parseInput(line);         
        }

        if (curr_pcb->code_offset >= CODE_MEM_SIZE || !curr_pcb->code[curr_pcb->code_offset]) {
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
    pcb_t *curr_pcb;
    int error_code = 0;

    ready_queue_reorder_sjf();

    while (get_ready_queue_size() > 0) {
        curr_pid = ready_queue.head->pid;
        curr_pcb = pcb_array[curr_pid];

        if (curr_pcb->code_offset < CODE_MEM_SIZE && curr_pcb->code[curr_pcb->code_offset]) {
            line = curr_pcb->code[curr_pcb->code_offset];
            error_code = parseInput(line);
            curr_pcb->code_offset++;
        }

        if (curr_pcb->code_offset >= CODE_MEM_SIZE || !curr_pcb->code[curr_pcb->code_offset]) {
            ready_queue_pop(&curr_pid);
            free_script_memory_at_index(curr_pid);
            free_pcb_for_pid(curr_pid);
        }
        ready_queue_reorder_aging(curr_pid);
    }

    return error_code;
}
