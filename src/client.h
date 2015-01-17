#ifndef __KEYSTORE_CLIENT__
#define __KEYSTORE_CLIENT__

int store_set(char key[], char *value, int num_dbs, char *dbs[]);
int store_get(char key[], int num_dbs, char *dbs[]);
int store_halt();

#endif
