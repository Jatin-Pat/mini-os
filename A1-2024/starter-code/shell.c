#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
    printf("Shell version 1.3 created September 2024\n");
    help();

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }
    
    //init shell memory
    mem_init();
    while(1) {							
        if (isatty(0)){
            printf("%c ", prompt);
        } else if (feof(stdin)) {
            mem_deinit();
            break;
        }
        // here you should check the unistd library 
        // so that you can find a way to not display $ in the batch mode
        fgets(userInput, MAX_USER_INPUT-1, stdin);
        errorCode = parseInput(userInput);
        if (errorCode == -1) exit(99);	// ignore all other errors
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ' || c == ';';
}

int parseInput(char inp[]) {
    char tmp[200];
    char *words[100];
    int ix = 0;
    int w = 0;
    int wordlen = 0;
    int errorCode = 0;
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        for (; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces

        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];
        }
        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        w++;

        if (inp[ix] != ' ') { // if white space, continue processing command
            errorCode = interpreter(words, w) || errorCode;

            for (int i = 0; i < w; i++) {
                free(words[i]);
                words[i] = 0;
            }

            if (inp[ix] == '\n' || inp[ix] == '\0') break;
            // when ';' is encountered, reset word count and increment ix to skip over char
            w = 0;
            ix++;
        }
    }
    return errorCode;
}