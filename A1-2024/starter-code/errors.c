#include <stdio.h>

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For run command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int badcommandTooManyTokens() {
    printf("Bad command: Too many tokens\n");
    return 4;
}

int badcommandTooFewTokens() {
    printf("Bad command: Too few tokens\n");
    return 5;
}

int badcommandDirectoryAlreadyExist() {
    printf("Bad command: Directory already exists\n");
    return 6;
}

int exceptionCannotLoadInteractiveScript() {
    printf("An exception occurred: Cannot load a script into memory when not in batch mode.\n");
    return 7;
}
