/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#ifndef __KEYSTORE_CLIENT__
#define __KEYSTORE_CLIENT__

int store_set(char key[], char *value, int num_dbs, char *dbs[]);
int store_get(char key[], int num_dbs, char *dbs[]);
int store_halt();

#endif
