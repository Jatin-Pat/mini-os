#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#include "shell.h"
#include "interpreter.h"
#include "codememory.h"
#include "schedulermemory.h"
#include "setup.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
    char * mychar = malloc(sizeof(char));
    if (!mychar) {
        printf("null malloc");
    }
    printf("hi %s\n", mychar);
    printf("Shell version 1.3 created September 2024\n\n");

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

/**
* Determines if a character is a word ending character.
*
* @param c the character to check 
* @return:
*   - 0 if c is not a word ending character
*   - 1 if c is a word ending character
*/
int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ' || c == ';';
}

/**
* Parses the input into words and interprets them.
*
* @param inp the input to parse
* @return:
*   - 0 if success
*   - error code when not ok
*/
int parseInput(char inp[]) {
    char tmp[200];
    char *words[100];
    int ix = 0;
    int w = 0;
    int wordlen = 0;
    int errorCode = 0;
    for (; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        // extract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];                        
        }
        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        w++;
        if (inp[ix] == '\0') break;
        else if (inp[ix] == ';') { // process chained command
            errorCode = interpreter(words, w) || errorCode; // if chaining and a command fails, keep that error code

            for (int i = 0; i < w; i++) {
                free(words[i]);
                words[i] = 0;
            }
            w = 0;
            ix++; // skip over ';' character
        }

        ix++;
    }

    // process last command 
    errorCode = interpreter(words, w) || errorCode;

    for (int i = 0; i < w; i++) {
        free(words[i]);
        words[i] = 0;
    }
    return errorCode;
}
