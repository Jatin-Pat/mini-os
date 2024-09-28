#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7;

int badcommand(){
    printf("Unknown Command\n");
    return 1;
}

// For run command only
int badcommandFileDoesNotExist(){
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

int badcommand();
int badcommandFileDoesNotExist();
int badcommandTooManyTokens();
int badcommandTooFewTokens();
int help();
int quit();
int set(char* command_args[], int args_size);
int print(char* var);
int run(char* script);
int echo(char* arg);

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size) {
    int i;

    if (args_size < 1) {
        return badcommand();
    } else if (args_size > MAX_ARGS_SIZE) {
        return badcommandTooManyTokens();
    }

    for (i = 0; i < args_size; i++) { // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0){
        //help
        if (args_size != 1) return badcommand();
        return help();
    
    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1) return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size < 3) return badcommand();
        else if (args_size > 7) return badcommandTooManyTokens();
        return set(command_args, args_size);
    
    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2) return badcommand();
        return print(command_args[1]);
    
    } else if (strcmp(command_args[0], "run") == 0) {
        if (args_size != 2) return badcommand();
        return run(command_args[1]);
    
    } else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2) return badcommand();
        return echo(command_args[1]);

    } else return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *command_args[], int num_args) {
    char *var = command_args[1];
    size_t size_of_value = sizeof(char) * num_args * 100;
    char *value = malloc(size_of_value);
    memset(value, '\0', size_of_value);
    int value_size = 0;

    for (int i = 2; i < num_args; i++){
        strcat(value, command_args[i]);
        strcat(value, " ");
        value_size += (strlen(command_args[i]) + 1);
    }
    value[value_size - 1] = '\0'; // remove last space

    mem_set_value(var, value);
    free(value);
    return 0;
}

int print(char *var) {
    char *value = mem_get_value(var);
    if (value) {
        printf("%s\n", value);
    } else {
        printf("Variable does not exist\n");
    }
    free(value);
    return 0;
}

int run(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");  // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT-1, p);
    while (1) {
        errCode = parseInput(line);	// which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT-1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char *arg) {
    int error_code = 0;

    if (arg[0] == '\0') {
        return badcommandTooFewTokens();
    } else if (arg[0] == '$') {
        size_t sizeof_var = sizeof(char) * strlen(arg);
        char *var = malloc(sizeof_var);
        memset(var, '\0', sizeof_var);
        strcpy(var, (arg + 1)); // skip the '$' char

        char *value = mem_get_value(var);
        if (value) {
            printf("%s\n", value);
        } else {
            printf("\n");
        }
        free(value);
        free(var);

    } else {
        printf("%s\n", arg);
    }
    return error_code;
}
