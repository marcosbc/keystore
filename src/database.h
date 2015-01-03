/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund and Adrian Marcelo Anillo
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#ifndef __KEYSTORE_DATABASE__

#include "types.h"

store_db *create_db(char name[MAX_DB_SIZE], store_db **dbs);
store_entry *create_entry(char key[MAX_KEY_SIZE], store_db **db);
store_entry *locate_entry(char path[MAX_KEY_SIZE], store_db *db);
store_db *locate_db(char *name, store_db *db);

#ifdef __DEBUG__
void print_databases(int num_dbs, char *dbs[]);
void print_existing_databases(store_db *store_dbs);
void print_store_tree(store_db **db);
#endif

#endif
