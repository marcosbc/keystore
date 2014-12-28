/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_STORE__
#define __KEYSTORE_STORE__

#include <sys/un.h>

// pipe paths
#define SERVER_SOCKET_ADDRESS "keystore_server.sock"

void store_stop();
store_entry *store_write(char key[MAX_KEY_SIZE], char *val, int num_dbs,
                         char *dbs, store_info **store);
store_entry *store_read(char key[MAX_KEY_SIZE], int num_dbs, char *dbs,
                        store_info *store);
int store_server_act(int s, store_info **store);
int store_server_init();

#ifdef __DEBUG__
void print_databases(int num_dbs, char *dbs[]);
void print_existing_databases(store_db *store_dbs);
#endif

#endif