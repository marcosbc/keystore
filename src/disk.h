/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#ifndef __KEYSTORE_DISK__
#define __KEYSTORE_DISK__

#include "types.h"

void *disk_set(void *ent);
void *disk_get(void *ent);

#endif
