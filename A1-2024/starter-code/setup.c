#include "shellmemory.h"
#include "schedulermemory.h"

int deinit() {
    mem_deinit();
    process_code_mem_deinit();
    return 0;
}
