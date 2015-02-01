#ifndef __KEYSTORE_COMMON__
#define __KEYSTORE_COMMON__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

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

// common error types (no. 1 is always ERR_USE)
#define ERR_NONE 0
#define ERR_USE 1
#define ERR_ALLOC 2
#define ERR_ALLOC_MSG "memory allocation couldn't be done"
#define ERR_SHMAT 3
#define ERR_SHMAT_MSG "while mapping shared memory segment"
#define ERR_SHMDT 4
#define ERR_SHMDT_MSG "couldn't unmap shared memory"
#define ERR_SEMOPEN 5
#define ERR_SEMOPEN_MSG "while opening semaphores"
#define ERR_SEMCLOSE 6
#define ERR_SEMCLOSE_MSG "while closing semaphores"
#define ERR_SOCKET 7
#define ERR_SOCKET_MSG "couldn't create socket"
// #define ERR_CHK 9
// #define ERR_CHK_MSG "checksum doesn't match"

// server modes
#define STORE_NUM_MODES 3
#define STORE_MODE_SET_ID 0
#define STORE_MODE_GET_ID 1
#define STORE_MODE_STOP_ID 2

// socket path max size
#define MAX_SOCK_PATH_SIZE 100

// shared memory key
#define KEY_ID 'K'

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
	//   techically not needed thanks to '\0'
	// - a simulated array of values (single-pointer array of strings)
};

double time_diff(struct timeval start, struct timeval end);
void print_error(const char *msg, ...);
void print_perror(const char *msg);
int max(int x, int y);
int min(int x, int y);

#endif
