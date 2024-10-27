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
int exec(char *command_args[], int num_args);

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
        if (args_size != 1) return badcommand();
        return help();
    
    } else if (strcmp(command_args[0], "quit") == 0) {
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

    } else if (strcmp(command_args[0], "exec") == 0) {
        if (args_size < 3) return badcommand();
        else if (args_size > 5) return badcommandTooManyTokens();
        return exec(command_args, args_size);
    } 
    
    else return badcommand();
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

/**
* Sets a variable to a value.
*
* @param:
*   - command_args: an array of the variable name and the value to set
*   - num_args: the number of arguments in command_args
* @return:
*   - 0 if success
*/
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

/**
* Prints the value of a variable.
*
* @param var: the name of the variable to print
* @return:
*   - 0 if success
*/
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

/**
* Runs a script file with the given name.
*
* @param script: the name of the script file to run
* @return:
*   - 0 if success
*/
int run(char *script) {
    int pid;
    int errCode = 0;
    
    errCode = find_free_pid(&pid);
    if (errCode) { return errCode; }

    int line_count;
    errCode = load_script_into_memory(script, pid, &line_count);
    if (errCode) { return errCode; }

    errCode = create_pcb_for_pid(pid, line_count);
    if (errCode) { return errCode; }

    ready_queue_push(pid);
    char *policy = "FCFS";

    errCode = run_scheduler(policy);

    return errCode;
}

/**
* display strings passed as argument to the command line.
*
* @param arg: the string to display
* @return:
*   - 0 if success
*/
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

/**
* Lists all the files present in the current directory.
*
* @return:
*   - 0 if success
*/
int my_ls() {
    struct dirent **namelist;
    int n;

    n = scandir(".", &namelist, NULL, alphasort);
    
    // Since scandir also contains . and .. entries, which we omit, we start from index 2.
    for (int i = 2; i < n; i++) {
        printf("%s\n", namelist[i]->d_name);
        free(namelist[i]);
    }

    // must free . and .. entries we skipped.
    free(namelist[0]);
    free(namelist[1]);
    free(namelist);
    return 0;
}

/**
* Creates a new empty file inside the current directory.
*
* @param filename: the name of the file to create
* @return:
*   - 0 if success
*/
int my_touch(char *filename) {

    if (filename[0] == '\0') {
        return badcommandTooFewTokens();
    }
    // fopen creates the file if it does not exist
    FILE *file = fopen(filename, "w");
    fclose(file);
    return 0;
}

/**
* Creates a new directory with the name dirname in the current directory.
*
* @param dirname: the name of the directory to create
* @return:
*   - 0 if success
*/
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

/**
* Changes current directory to directory specified in dirname.
*
* @param dirname: the directory to change to
* @return:
*   - 0 if success
*/
int my_cd(char *dirname) {
    
    if (dirname[0] == '\0') {
        return badcommandTooFewTokens();
    }

    if (chdir(dirname) != 0) {
        printf("Bad command: my_cd\n");
    }

    return 0;
}

/**
* Executes up to 3 concurrent programs, according to a given scheduling policy.
* 
* @param:
*   - command_args: an array of program names and a scheduling policy
*   - num_args: the number of arguments in command_args
* @return:
*   - 0 if success
*/
int exec(char *command_args[], int num_args) {
    char *policy = command_args[num_args - 1];

    if (strcmp(policy, "FCFS") != 0 &&
        strcmp(policy, "SJF") != 0 &&
        strcmp(policy, "RR") != 0 &&
        strcmp(policy, "AGING") != 0) {
        return badcommandInvalidPolicy();
    }

    for (int i = 1; i < num_args - 1; i++) {
        for (int j = i + 1; j < num_args - 1; j++) {
            if (strcmp(command_args[i], command_args[j]) == 0) {
                return badcommandDuplicateProgramsInExec();
            }
        }
    }

    int errCode = 0; 
    for (int i = 1; i < num_args - 1; i++) {
        int pid;

        errCode = find_free_pid(&pid);
        if (errCode) { return errCode; }

        // load_script_into_memory() will set line_count with the number of lines in the script
        // and the line_count will be used to set the job_length_score in create_pcb_for_pid().
        int line_count;
        errCode = load_script_into_memory(command_args[i], pid, &line_count);
        if (errCode) { return errCode; }

        errCode = create_pcb_for_pid(pid, line_count);
        if (errCode) { return errCode; }

        ready_queue_push(pid);
    }

    errCode = run_scheduler(policy);

    return errCode;
}
