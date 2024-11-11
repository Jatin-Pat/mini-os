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
