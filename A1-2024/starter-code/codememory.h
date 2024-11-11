#ifndef CODEMEMORY_H
#define CODEMEMORY_H

typedef struct {
    char *backing_store_fname;
    int entries[MAX_PAGE_TABLE_ENTRIES];
} page_table_t;


#endif
