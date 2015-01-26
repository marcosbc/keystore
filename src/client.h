#ifndef __KEYSTORE_CLIENT__
#define __KEYSTORE_CLIENT__

int store_set(char key[], char *value, int num_dbs, char *dbs[]);
int store_get(char key[], int num_dbs, char *dbs[]);
int store_halt();
int store_act(int s, struct request_info *req_inf, struct request **req,
              struct response_info *res_inf, struct response **res);

#endif
