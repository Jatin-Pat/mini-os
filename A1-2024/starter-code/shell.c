#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#include "codememory.h"
#include "interpreter.h"
#include "resourcemanager.h"
#include "scheduler.h"
#include "schedulermemory.h"
#include "setup.h"
#include "shell.h"
#include "shellmemory.h"

// Start of everything
int main(int argc, char *argv[]) {
    printf("Frame Store Size = %d; Variable Store Size = %d\n", CODE_MEM_SIZE, VAR_MEM_SIZE);

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }
    
    // init scheduler memory
    errorCode = process_code_mem_init();
    if (errorCode) {
        printf("ERROR when initializing process code memory. Exiting.");
        exit(99);
    }

    //init shell memory
    mem_init();
    while(1) {							
        if (isatty(0)){
            printf("%c ", prompt);
        } else if (feof(stdin)) {
            deinit();
            break;
        }
        fgets(userInput, MAX_USER_INPUT-1, stdin);
        
        errorCode = parseInput(userInput);
        if (errorCode == -1) exit(99);	// ignore all other errors
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

