#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

// Helper functions
int match(char *model, char *var) {
    int len = strlen(var);
    int matchCount = 0;
    for (int i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    return (matchCount == len);
}

// Shell memory functions

void mem_init() {
    for (int i = 0; i < MEM_SIZE; i++){
        shellmemory[i].var   = NULL;
        shellmemory[i].value = NULL;
    }
}

void mem_deinit() {
    for (int i = 0; i < MEM_SIZE; i++) {
        free(shellmemory[i].var);
        free(shellmemory[i].value);

        shellmemory[i].var = NULL;
        shellmemory[i].value = NULL;
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (shellmemory[i].var && strcmp(shellmemory[i].var, var_in) == 0){
            free(shellmemory[i].value);
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++){
        if (!shellmemory[i].var){
            shellmemory[i].var   = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (shellmemory[i].var && strcmp(shellmemory[i].var, var_in) == 0){
            return strdup(shellmemory[i].value);
        } 
    }
    return strdup("Variable does not exist");
}
