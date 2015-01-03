/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund and Adrian Marcelo Anillo
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#ifndef __KEYSTORE__
#define __KEYSTORE__

#define ERR_USE 1
#define ERR_USE_MSG "use: %s [set KEY VALUE|get KEY] DB1[ DB2[ ...]]\n"

#define MODE                1
#define MODE_SET            "set"
#define MODE_SET_ID         1
#define MODE_GET            "get"
#define MODE_GET_ID         2
#define MODE_HALT           "stop"
#define MODE_HALT_ID        0

#define INIT_NUM_ARGS       2 // 1 + 1

#define SET_MIN_NUM_ARGS    5 // 4 + 1
#define SET_KEY             2
#define SET_VAL             3
#define SET_NAMES           4

#define GET_MIN_NUM_ARGS    4 // 3 + 1
#define GET_KEY             2
#define GET_NAMES           3

#define HALT_NUM_ARGS       2 // 1 + 1

int check_arguments(int num, char *args[], int *mode);

#endif
