#ifndef __KEYSTORE_COMMON__
#define __KEYSTORE_COMMON__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define __DEBUG__
#ifdef __DEBUG__
#define DEBUG_PRINT(...) \
{ \
	fprintf(stderr, "\033[33m"); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\033[0m"); \
	\
}
#else
#define DEBUG_PRINT(...) {}
#endif

#define KEY_ID 'K'

#define ERR_NONE 0
#define ERR_SESSION 2
#define ERR_SESSION_MSG "you already have a session running"
#define ERR_ALLOC 5
#define ERR_ALLOC_MSG "memory allocation returned NULL"
#define ERR_STORE_SHMCREATE 10
#define ERR_STORE_SHMCREATE_MSG "couldn't create shared memory"
#define ERR_STORE_SHMLOAD 11
#define ERR_STORE_SHMLOAD_MSG "couldn't map shared memory"
#define ERR_STORE_SHMAT 12
#define ERR_STORE_SHMAT_MSG "while mapping shared memory segment"
#define ERR_STORE_SHMDT 13
#define ERR_STORE_SHMDT_MSG "couldn't unmap shared memory"
#define ERR_STORE_SHMCTL 14
#define ERR_STORE_SHMCTL_MSG "while removing our shared memory segment"
#define ERR_MEM_SEMOPEN 15
#define ERR_MEM_SEMOPEN_MSG "while opening semaphores"
#define ERR_MEM_SEMUNLINK 16
#define ERR_MEM_SEMUNLINK_MSG "while unlinking semaphores"
#define ERR_MEM_SEMCLOSE 17
#define ERR_MEM_SEMCLOSE_MSG "while closing semaphores"
#define ERR_THR 20
#define ERR_THR_MSG ""
#define ERR_THRJOIN 21
#define ERR_THRJOIN_MSG "error in thread join"
#define ERR_DB 30
#define ERR_DB_MSG "db not found"
#define ERR_ENTRY 31
#define ERR_ENTRY_MSG "entry not found"
#define ERR_SOCKETEXIST 40
#define ERR_SOCKETCREATE 41
#define ERR_BIND 42
#define ERR_LISTEN 43
#define ERR_ACCEPT 44
#define ERR_CONNECT 44
#define ERR_SIZE 45 // actually it is a flag

#define STORE_NUM_MODES 3
#define STORE_MODE_SET_ID 0
#define STORE_MODE_GET_ID 1
#define STORE_MODE_STOP_ID 2

#define MAX_SOCK_PATH_SIZE 100

// for extracting info from shared memory
typedef struct info {
	pid_t pid;
	int max_sock_len;
	char sock_path[MAX_SOCK_PATH_SIZE];
	int max_key_len;
	int max_val_len;
	int max_db_len;
	char modes[STORE_NUM_MODES];
} store_info;

/*
struct entry {
	char *key;
	int val_len;
	char *value;
};
*/

struct request_info {
	char mode;
	size_t size;
};

struct request {
	int num_dbs;
	int val_size;

	// the request will also include right after this:
	// - a string for the key
	// - a string for the value (if any)
	// - a simulated array of the databases (single-pointer array of values)
};

struct response_info {
	int error; // if error is 0, it went ok
	size_t size; // size of the response
};

// dbs not needed (results are ordered), key not yet
struct response {
	int num; // number of values to send

	// the response will also include right after this:
	// - an array of integers of the size of each val: int val_size[num]
	// - a simulated array of values (single-pointer array of strings)
	// techically not needed thanks to '\0'
};

void print_error_case(int error);
void print_error(const char *msg, ...);
void print_perror(const char *msg);
int max(int x, int y);
int min(int x, int y);

#endif
