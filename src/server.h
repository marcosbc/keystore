/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_STORE__
#define __KEYSTORE_STORE__

#include <sys/un.h>

void store_stop();
store_entry **store_write(char key[MAX_KEY_SIZE], char *val, int num_dbs,
                          char *db_names, store_db **dbs);
store_entry **store_read(char key[MAX_KEY_SIZE], int num_dbs, char *db_names,
                         store_db *dbs);
int store_server_act(int s, store_db **dbs);
int store_server_init();

#endif
