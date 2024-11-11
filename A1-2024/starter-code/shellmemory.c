#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "setup.h"
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[VAR_MEM_SIZE];

pthread_mutex_t shellmemory_lock = PTHREAD_MUTEX_INITIALIZER;

/**
* Compares two strings to see if they are equal.
*
* @param:
*   - model: the string to compare against
*   - var: the string to compare
* @return:
*   - 1 if the strings are equal
*   - 0 if the strings are not equal
*/
int match(char *model, char *var) {
    int len = strlen(var);
    int matchCount = 0;
    for (int i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    return (matchCount == len);
}

/**
* Initializes the shell memory.
*/
void mem_init() {
    for (int i = 0; i < VAR_MEM_SIZE; i++){
        shellmemory[i].var   = NULL;
        shellmemory[i].value = NULL;
    }
}

/**
* Deinitializes the shell memory.
*/
void mem_deinit() {
    for (int i = 0; i < VAR_MEM_SIZE; i++) {
        free(shellmemory[i].var);
        free(shellmemory[i].value);

        shellmemory[i].var = NULL;
        shellmemory[i].value = NULL;
    }
}

/**
* Sets key value pair in shell memory.
*
* @param:
*   - var_in: the key to set
*   - value_in: the value to set
*/
void mem_set_value(char *var_in, char *value_in) {
    int i;

    pthread_mutex_lock(&shellmemory_lock);
    for (i = 0; i < VAR_MEM_SIZE; i++){
        if (shellmemory[i].var && strcmp(shellmemory[i].var, var_in) == 0){
            free(shellmemory[i].value);
            shellmemory[i].value = strdup(value_in);
            pthread_mutex_unlock(&shellmemory_lock);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < VAR_MEM_SIZE; i++){
        if (!shellmemory[i].var){
            shellmemory[i].var   = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            pthread_mutex_unlock(&shellmemory_lock);
            return;
        } 
    }

    pthread_mutex_unlock(&shellmemory_lock);
    return;
}

/**
* Gets the value of a key in shell memory.
*
* @param:
*   - var_in: the key to get the value of
* @return:
*   - the value of the key
*   - NULL if the key does not exist
*/
char *mem_get_value(char *var_in) {
    int i;

    pthread_mutex_lock(&shellmemory_lock);
    for (i = 0; i < VAR_MEM_SIZE; i++){
        if (shellmemory[i].var && strcmp(shellmemory[i].var, var_in) == 0){
            pthread_mutex_unlock(&shellmemory_lock);
            return strdup(shellmemory[i].value);
        } 
    }
    
    pthread_mutex_unlock(&shellmemory_lock);
    return NULL;
}
