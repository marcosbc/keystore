/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
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
