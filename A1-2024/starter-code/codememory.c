#include <stddef.h>

#include "setup.h"

#include "codememory.h"

char **code_mem;
char *free_frames;

page_table_t *page_table_array[MAX_NUM_PROCESSES] = {NULL};
