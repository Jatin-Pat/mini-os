#include "codememory.h"
#include "shellmemory.h"
#include "schedulermemory.h"

#include "resourcemanager.h"

/**
* Deinitializes the shell memory.
*
* @return:
*   - 0 if success
*/
int deinit() {
    mem_deinit();
    code_mem_deinit();
    return 0;
}
