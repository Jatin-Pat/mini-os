#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"
#include "setup.h"

#include "codememory.h"

int allocate_frame_to_page(int pid, int page_num);
int find_page_table_with_fname(int pid, char *fname);
int get_pt_entry_for_line(int pid, int codeline);
char *get_backstore_fname_for_pid(int pid);
int load_page_at(int pid, int codeline);

char **code_mem;
char *free_frames;
int next_page_load = 0;

page_table_t *page_table_array[MAX_NUM_PROCESSES] = {NULL};

/**
* Initializes the process code memory.
* @return: 
*   - 0
*/
int code_mem_init() {
    code_mem = malloc(20 * sizeof(char*));

    free_frames = (char *) malloc(num_frames() * sizeof(char));
    memset(free_frames, 1, num_frames() * sizeof(char));  // all frames initially free
    return 0;
}

/**
* Deinitializes the process code memory.
* @return:
*   - 0
*/
int code_mem_deinit() {
    free(code_mem);
    code_mem = NULL;

    free(free_frames);
    free_frames = NULL;
    return 0;
}

// TODO REWORK
/**
* Frees the memory allocated for a script at an index.
*
* @param index the index of the script to free
* @return:
*   - 0
*/
int free_script_memory() {
    int error_code = 0;
    char *pline;

    for (int i = 0; i < CODE_MEM_SIZE; i++) {
        pline = code_mem[i];
        if (pline) {
            free(pline);
            code_mem[i] = NULL;
        } 
    }    
    return error_code;
}

int find_page_table_with_fname(int pid, char *fname) {
    page_table_t *pt;
    for (int i; i < MAX_NUM_PROCESSES; i++) {
         pt = page_table_array[i];
        if (pt && strcmp(pt->backing_store_fname, fname) == 0) {
           return i;         
        }
    }
    return pid;
}

int create_page_table_for_pid(int pid, char *backing_store_fname) {
    if (page_table_array[pid]) {
        return 1; //TODO BETTER ERROR: PT array nonvoid
    }

    page_table_t *curr_pt;
    int page_table_index = find_page_table_with_fname(pid, backing_store_fname);
    if (page_table_index == pid) {
        curr_pt = malloc(sizeof(page_table_t));
        curr_pt->backing_store_fname = backing_store_fname;
        memset(curr_pt, -1, sizeof(curr_pt->entries)); // set all as invalid
    } else {
        curr_pt = page_table_array[page_table_index];
    }

    page_table_array[pid] = curr_pt;

    return 0;
}

int free_page_table_for_pid(int pid) {
    page_table_t *pt = page_table_array[pid];
    page_table_array[pid] = NULL;

    // if there exists another page table which uses the same file,
    // don't free that memory, just remove the current pointer to it
    if (find_page_table_with_fname(pid, pt->backing_store_fname) != pid) {
        free(pt);
    }
    return 0; 
}

int get_pt_entry_for_line(int pid, int codeline){
    int pte_index = floor(codeline / PAGE_SIZE);
    return page_table_array[pid]->entries[pte_index];
}

int allocate_frame_to_page(int pid, int page_num) {
    for (int i = 0; i < num_frames(); i++) {
        if (free_frames[i]) {
            page_table_array[pid]->entries[page_num] = i;
            return 0;
        }
    }
    // TODO OUT OF MEMORY ERROR 
    return 1;
}

char *get_backstore_fname_for_pid(int pid) {
    return page_table_array[pid]->backing_store_fname;
}

int load_page_at(int pid, int codeline) {
    int error_code = 0;
    int memory_addr;
    char line[MAX_USER_INPUT];
   
    // check if invalid entry
    int page_num = floor(codeline / PAGE_SIZE);
    int frame_number = get_pt_entry_for_line(pid, codeline);
    if (frame_number == -1) {
        error_code = allocate_frame_to_page(pid, page_num);
        // TODO OOM
        if (error_code) { return 1; }
        frame_number = get_pt_entry_for_line(pid, codeline);
    }

    char *filename = get_backstore_fname_for_pid(pid);
    FILE *p = fopen(filename, "rt");
    if (!p) {
        return badcommandFileDoesNotExist();
    }

    // skip until codeline
    for (int i = 0; i < codeline && fgets(line, MAX_USER_INPUT, p); i++) {
       if (feof(p)) { break; }
    }  
    memset(line, 0, sizeof(line));

    // load code into frame
    for (int i = 0; i < PAGE_SIZE; i++) {
        memory_addr = (frame_number * PAGE_SIZE) + i;
        code_mem[memory_addr] = NULL;
        if (fgets(line, MAX_USER_INPUT, p) && !feof(p)) {
            code_mem[memory_addr] = line;
            memset(line, 0, sizeof(line));
        }
    }

    fclose(p);

    next_page_load = (next_page_load + 1) % num_frames();
    
    return 0; 
}

int get_memory_at(int pid, int codeline, char **line) {
    int error_code = 0;
    int memory_addr;
   
    // check if invalid entry
    int offset = codeline % PAGE_SIZE;
    int frame_number = get_pt_entry_for_line(pid, codeline);
    if (frame_number == -1) {
        //TODO PAGEFAULT!!! (shouldn,t happen in 1.2.1)
        // handle pagefault
    }
    
    memory_addr = (frame_number * PAGE_SIZE) + offset;
    *line = code_mem[memory_addr];
    
    return error_code; 
}

// TODO REWORK
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
int load_script_into_memory(int pid, int *line_count) {
    int error_code = 0;
    char *filename = get_backstore_fname_for_pid(pid);
    FILE *p = fopen(filename, "rt");
    *line_count = count_lines_in_file(p);
    
    for (int i = 0; i < *line_count; i += PAGE_SIZE) {
        load_page_at(pid, i);
    } 

    return error_code;
}


// TODO REWORK
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
    int error_code = 0;

    if (isatty(0)){
        return exceptionCannotLoadInteractiveScript();
    }

    int line_count = count_lines_in_file(stdin);
    
    for (int i = 0; i < line_count; i += 3) {
        load_page_at(pid, i);
    } 

    return error_code;
}

