/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_CLIENT__
#define __KEYSTORE_CLIENT__

int store_set(char key[], char *value, int num_dbs, char *dbs[]);
int store_get(char key[], int num_dbs, char *dbs[]);
int store_halt();

#endif
