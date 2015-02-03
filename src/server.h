#ifndef __KEYSTORE_SERVER__
#define __KEYSTORE_SERVER__

#include <sys/un.h>
#include "types.h"

#define MSG_CONN_INCOMING "connection incoming\n"
#define MSG_REQUEST_TIME_ELAPSED "connection closed after %.3f msec\n"
#define MSG_RUNNING "server running, took %.3f msec to start\n"
#define MSG_STOPPING "stopping server...\n"
#define MSG_STOPPED "server stopped after running for %.3f sec\n"

void store_stop();
int store_write(char key[MAX_KEY_SIZE], char *val, int num_dbs,
                char *db_names, store_db **dbs, store_entry ***result);
int store_read(char key[MAX_KEY_SIZE], int num_dbs, char *db_names,
               store_db *dbs, store_entry ***result);
int store_server_act(int s, store_db **dbs);
int store_server_init();

#endif
