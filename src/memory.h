/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#ifndef __KEYSTORE_MEMORY__
#define __KEYSTORE_MEMORY__

#include <semaphore.h>
#include "types.h"

int memory_init();
void *memory_set(void *info);
void *memory_get(void *info);
int memory_clear(store_db **dbs);
int free_tree(store_db **dbs);

#endif
