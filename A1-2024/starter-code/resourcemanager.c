#include "shellmemory.h"
#include "schedulermemory.h"

/**
* Deinitializes the shell memory.
*
* @return:
*   - 0 if success
*/
int deinit() {
    mem_deinit();
    process_code_mem_deinit();
    return 0;
}
