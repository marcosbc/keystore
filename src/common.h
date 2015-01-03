/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund and Adrian Marcelo Anillo
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

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
#define ERR_FORK 3
#define ERR_FORK_MSG "fork returned a number less than zero"
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
#define ERR_PID_NUM 35
#define ERR_PID_NUM_MSG "pid has wrong format"
#define ERR_PID_EXIST 36
#define ERR_PID_EXIST_MSG "the pid file already exists"
#define ERR_PID_CREATE 37
#define ERR_PID_CREATE_MSG "couldn't create the pid file: check permissions"
#define ERR_FIFO_CREATE 38
#define ERR_FIFO_CREATE_MSG "the fifo pipe couldn't be created"
#define ERR_FIFO_OPEN 39
#define ERR_FIFO_OPEN_MSG "the fifo pipe couln't be opened"
#define ERR_SOCKETEXIST 40
#define ERR_SOCKETCREATE 41
#define ERR_BIND 42
#define ERR_LISTEN 43
#define ERR_ACCEPT 44
#define ERR_CONNECT 44

#define STORE_NUM_MODES 3
#define STORE_MODE_SET_ID 0
#define STORE_MODE_GET_ID 1
#define STORE_MODE_STOP_ID 2

#define STORE_ACK_LEN 3 // strlen('o', 'k') + 1 ('\0')
#define MAX_SOCK_PATH_SIZE 100

// for extracting info from shared memory
typedef struct info {
	pid_t pid;
	int ack_len;
	char ack_msg[STORE_ACK_LEN];
	int max_sock_len;
	char sock_path[MAX_SOCK_PATH_SIZE];
	int max_key_len;
	int max_val_len;
	int max_db_len;
	char modes[STORE_NUM_MODES];
} store_info;

void print_error_case(int error);
void print_error(const char *msg, ...);
void print_perror(const char *msg);
int max(int x, int y);
int min(int x, int y);

#endif
