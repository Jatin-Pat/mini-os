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
int create_process_from_filename(char *filename, int *ppid);
int create_process_from_current_file(int *ppid);

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

/**
* Executes the scripts passed according to a policy. Can be run in the background, or multithreaded.
* 
* @param command_args The command line arguments with which exec was called
* @param num_args The size of command_args. 
*
* @return 0 for success, or an error code.
*/
int exec(char *command_args[], int num_args) {
    char *policy;
    int policy_index = num_args - 1;
    char executes_in_background = 0;
    char executes_multithreaded = 0;
    int error_code = 0; 

    if (strcmp(command_args[policy_index], "MT") == 0) {
        executes_multithreaded = 1;
        policy_index -= 1;
    }

    if (*command_args[policy_index] == '#') {
        executes_in_background = 1;
        policy_index -= 1; 
    }

    policy = command_args[policy_index];

    if (strcmp(policy, "FCFS") != 0 &&
        strcmp(policy, "SJF") != 0 &&
        strcmp(policy, "RR") != 0 &&
        strcmp(policy, "AGING") != 0) {
        return badcommand();
    }

    // throw an error if two scripts have the same name
    for (int i = 1; i < policy_index; i++) {
        for (int j = i + 1; j < policy_index; j++) {
            if (strcmp(command_args[i], command_args[j]) == 0) {
                return badcommand();
            }
        }
    }

    int pid;
    
    if (executes_in_background) { 
        error_code = create_process_from_current_file(&pid);
        ready_queue_prepend(pid);
    }

    for (int i = 1; i < policy_index; i++) {
        error_code = create_process_from_filename(command_args[i], &pid);
        if (error_code) { return error_code; }
        ready_queue_push(pid);
    }

    if (executes_multithreaded) {
       ; // TODO implement
    }
        

    error_code = run_scheduler(policy);

    // stop running after queue becomes empty: current process was run.
    if (executes_in_background) {
        deinit();
        exit(0);
    }

    return error_code;
}

/**
* Allocates a PCB for the process, and loads the script into memory.
*
* @param filename the filename of the file to load
* @param ppid a pointer to the pid. Gets updated with the pid of the new process.
*
* @return:
*   - 0 when ok
*   - error code when not ok
*/
int create_process_from_filename(char *filename, int *ppid) {
    int error_code = 0;
    int pid;
    int line_count;

    error_code = find_free_pid(&pid);
    if (error_code) { return error_code; }

    error_code = load_script_into_memory(filename, pid, &line_count);
    if (error_code) { return error_code; }

    error_code = create_pcb_for_pid(pid, line_count);
    if (error_code) { return error_code; }

    *ppid = pid;   
    return 0; 
}

/**
* Allocates a PCB for the process, and loads the script into memory.
*
* @param ppid a pointer to the pid. Gets updated with the pid of the new process.
*
* @return:
*   - 0 when ok
*   - error code when not ok
*/
int create_process_from_current_file(int *ppid) {
    int error_code = 0;
    int pid;

    error_code = find_free_pid(&pid);
    if (error_code) { return error_code; }

    error_code = create_pcb_for_pid(pid, 0);
    if (error_code) { return error_code; }

    error_code = load_current_script_into_memory(pid);
    if (error_code) { return error_code; }

    *ppid = pid;
    return 0;
} 
