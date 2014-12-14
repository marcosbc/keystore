/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE__
#define __KEYSTORE__

#define USE_MSG "use: ./keystore [set KEY VALUE|get KEY] DB1[ DB2[ ...]]"

#define MODE                1
#define MODE_INIT           "start"
#define MODE_INIT_ID        1
#define MODE_SET            "set"
#define MODE_SET_ID         2
#define MODE_GET            "get"
#define MODE_GET_ID         3
#define MODE_HALT           "stop"
#define MODE_HALT_ID        0

#define INIT_NUM_ARGS       2 // 1 + 1

#define SET_MIN_NUM_ARGS    6 // 5 + 1
#define SET_KEY             2
#define SET_VAL             3
#define SET_NAMES           4

#define GET_MIN_NUM_ARGS    5 // 4 + 1
#define GET_KEY             2
#define GET_NAMES           3

#define HALT_NUM_ARGS       2 // 1 + 1

int check_arguments(int num, char *args[], int *mode);

#endif
