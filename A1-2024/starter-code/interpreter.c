#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "errors.h"
#include "schedulermemory.h"
#include "setup.h"
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7;

int help();
int quit();
int set(char* command_args[], int args_size);
int print(char* var);
int run(char* script);
int echo(char* arg);
int my_ls();
int my_touch(char* filename);
int my_mkdir(char* dirname);
int my_cd(char* dirname);

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

    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1) return badcommand();
        return my_ls();

    } else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2) return badcommand();
        return my_touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2) return badcommand();
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2) return badcommand();
        return my_cd(command_args[1]);

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
    deinit();
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
    int pid;
    int errCode = 0;
    char line[MAX_USER_INPUT];
    
    errCode = find_free_pid(&pid);
    if (errCode) { return errCode; }

    errCode = load_script_into_memory(script, pid);
    if (errCode) { return errCode; }

    errCode = create_pcb_for_pid(pid);
    if (errCode) { return errCode; }

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
    printf("PID PID PID %d", pid);
    free_script_memory_at_index(pid);
    free_pcb_for_pid(pid);

    return errCode;
}

int echo(char *arg) {
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
    return 0;
}

int my_ls() {
    struct dirent **namelist;
    int n;

    n = scandir(".", &namelist, NULL, alphasort);
    
    for (int i = 2; i < n; i++) { // skip . and .. entries
        printf("%s\n", namelist[i]->d_name);
        free(namelist[i]);
    }

    free(namelist[0]);
    free(namelist[1]);
    free(namelist);
    return 0;
}

int my_touch(char *filename) {

    if (filename[0] == '\0') {
        return badcommandTooFewTokens();
    }
    FILE *file = fopen(filename, "w");
    fclose(file);
    return 0;
}

int my_mkdir(char *dirname) {
    int res = 0;

    if (dirname[0] == '\0') {
        return badcommandTooFewTokens();
    } else if (dirname[0] == '$') {
        size_t sizeof_var = sizeof(char) * strlen(dirname);
        char *var = malloc(sizeof_var);
        memset(var, '\0', sizeof_var);
        strcpy(var, (dirname + 1)); // skip the '$' char

        char *value = mem_get_value(var);
        if (value && strchr(value, ' ') == NULL) {
            res = mkdir(value, 0777);
        } else {
            printf("Bad command: my_mkdir\n");
        }

    } else {
        res = mkdir(dirname, 0777);
    }

    if (res == -1) {
        return badcommandDirectoryAlreadyExist();
    }

    return 0;
}

int my_cd(char *dirname) {
    
    if (dirname[0] == '\0') {
        return badcommandTooFewTokens();
    }

    if (chdir(dirname) != 0) {
        printf("Bad command: my_cd\n");
    }

    return 0;
}
