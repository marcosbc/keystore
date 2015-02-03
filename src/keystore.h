#ifndef __KEYSTORE__
#define __KEYSTORE__

// specific client error types
#define ERR_USE_MSG "use: %s [[[set KEY VALUE|get KEY] DB1[ DB2[ ...]]]|stop]\n"
#define ERR_SHMLOAD 30
#define ERR_SHMLOAD_MSG "couldn't load shared memory"
#define ERR_CONNECT 31
#define ERR_CONNECT_MSG "couldn't connect to the server"
#define ERR_SIZE 39
#define ERR_SIZE_MSG "the response size has an incorrect range"

// for command-line text input
#define MODE                1
#define MODE_SET            "set"
#define MODE_SET_ID         1
#define MODE_GET            "get"
#define MODE_GET_ID         2
#define MODE_HALT           "stop"
#define MODE_HALT_ID        0

// argument calculations of each mode
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
