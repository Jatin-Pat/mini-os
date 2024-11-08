#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"
#include "codememory.h"
#include "schedulermemory.h"
#include "shell.h"

char **process_code_memory;


/**
* Initializes the process code memory.
* @return: 
*   - 0
*/
int process_code_mem_init() {
    process_code_memory = (char **) calloc(MAX_NUM_PROCESSES * MAX_LINES_PER_CODE, sizeof(char*));
    }
    return 0;
}

/**
* Deinitializes the process code memory.
* @return:
*   - 0
*/
int process_code_mem_deinit() {
        free(process_code_memory);
        process_code_memory = NULL;
    }
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
int free_script_memory(int index) {
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


