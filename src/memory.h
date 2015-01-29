#ifndef __KEYSTORE_MEMORY__
#define __KEYSTORE_MEMORY__

#include <semaphore.h>
#include "types.h"

int memory_init();
void *memory_set(void *info);
void *memory_get(void *info);
int memory_clear(store_db **dbs);
void free_tree(store_db **dbs);

#endif
