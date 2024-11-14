#include <math.h>
#include <stdio.h>

#include "setup.h"

/**
* Returns the number of frames in code memory.
* 
* @return:
*  - the number of frames in code memory
*/
int num_frames() {
    return floor(CODE_MEM_SIZE / PAGE_SIZE);
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
* Returns the number of lines in file p.
*/
int count_lines_in_file(FILE *p) {
     char line[MAX_USER_INPUT];
    int current_line = 0;
    
    while(p) {
        if (feof(p)) {
            break;
        }
        if (!fgets(line, MAX_USER_INPUT, p)) {
            break;
        }
        current_line++;
    }

    return current_line;

}

