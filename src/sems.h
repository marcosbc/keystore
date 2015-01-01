/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#ifndef __KEYSTORE_SEM__
#define __KEYSTORE_SEM__

#include <semaphore.h>
#include "types.h"

// for shared memory RW (we want to be able to have multiple readers,
// but block them only if we are writing)
#define MAX_READERS_AT_ONCE 10
#define ONE                 1
#define SEM_MUTEX           "mutex"
#define SEM_RW              "readwrite"

int sems_init();
int sems_open();
int sems_close();
int sems_clear();
void read_lock();
void read_unlock();
void write_lock();
void write_unlock();

#endif
