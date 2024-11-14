#ifndef SETUP_H
#define SETUP_H

#include <stdio.h>

#define MAX_USER_INPUT 1000
#define MAX_NUM_PROCESSES 5
#define CODE_MEM_SIZE 18 
#define PAGE_SIZE 3
#define VAR_MEM_SIZE 10 

int num_frames();
int wordEnding(char c);
int count_lines_in_file(FILE *p);

#endif
