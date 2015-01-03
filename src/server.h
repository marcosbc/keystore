/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund and Adrian Marcelo Anillo
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#ifndef __KEYSTORE_SERVER__
#define __KEYSTORE_SERVER__

#include <sys/un.h>
#include "types.h"

void store_stop();
int store_write(char key[MAX_KEY_SIZE], char *val, int num_dbs,
                char *db_names, store_db **dbs, store_entry ***result);
int store_read(char key[MAX_KEY_SIZE], int num_dbs, char *db_names,
               store_db *dbs, store_entry ***result);
int store_server_act(int s, store_db **dbs);
int store_server_init();

#endif
