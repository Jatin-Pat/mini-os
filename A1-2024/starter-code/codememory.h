#ifndef CODEMEMORY_H
#define CODEMEMORY_H

#include "setup.h"

typedef struct {
    char *backing_store_fname;
    int *entries;
} page_table_t;


#endif
