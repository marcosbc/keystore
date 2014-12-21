/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_STORE__
#define __KEYSTORE_STORE__

int store_init();
int store_set(char key[], char value[], int num_dbs, char *dbs[]);
int store_get(char key[], int num_dbs, char *dbs[]);
int store_halt();

#ifdef __DEBUG__
void print_databases(int num_dbs, char *dbs[]);
void print_existing_databases(store_db *store_dbs);
#endif

#endif
