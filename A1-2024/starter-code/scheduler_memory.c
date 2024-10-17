#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "scheduler_memory.h"
#include "shell.h"

char *process_code_memory[MAX_NUM_PROCESSES][MAX_LINES_PER_CODE];

int process_code_mem_init() {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        for (int j = 0; j < MAX_LINES_PER_CODE; j++) {
            process_code_memory[i][j] = NULL;
        }
    }
    return 0;
}


/**
* Finds the first available code memory slot.
* Places the index of that slot in pointer pindex
* @return: 0 if success, 1 if out of slots
*/
int find_free_script_memory_index(int *pindex) {
    for (int i = 0; i < MAX_NUM_PROCESSES; i++) {
        if (!process_code_memory[i][0]) {
            *pindex = i;
            return 0;
        }
    }
    return 1;
}


int load_script_into_memory(char *filename, int *ppid) {
    char line[MAX_USER_INPUT];
    int error_code = 0;
    int current_line = 0;

    error_code = find_free_script_memory_index(ppid); 

    FILE *p = fopen(filename, "rt");
    if (!p) {
        return badcommandFileDoesNotExist();
    }

    // for each line in file, load into memory[free_index]
    fgets(line, MAX_USER_INPUT, p);
    while (1) {
        process_code_memory[*ppid][current_line] = strdup(line);
        current_line++;

        memset(line, 0, sizeof(line));
        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT, p);
    }

    fclose(p);
    return error_code;
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
