#include <math.h>

#include "setup.h"

/**
* Returns the number of frames in code memory.
* 
* @return:
*  - the number of frames in code memory
*/
int num_frames() {
    return floor(CODE_MEM_SIZE / 3);
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

