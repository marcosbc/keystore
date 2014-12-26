/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#ifndef __KEYSTORE_COMMON__
#define __KEYSTORE_COMMON__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG_PRINT(...) { fprintf(stderr, __VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#endif

#define KEY_ID 'K'
#define SHM_SIZE 1024

#define ERR_NONE 0
#define ERR_USE 1
#define ERR_USE_MSG "use: ./keystore [set KEY VALUE|get KEY] DB1[ DB2[ ...]]"
#define ERR_FORK 2
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

#define MAX_PATH_SIZE	1024
#define MAX_DB_SIZE		16
#define MAX_DICT_SIZE	16
#define MAX_KEY_SIZE	32
#define MAX_VAL_SIZE	512
#define COLLECTION_DELIMITER '.'

// for shared memory RW (we want to be able to have multiple readers,
// but block them only if we are writing)
#define MAX_READERS_AT_ONCE 10
#define ONE             1

#define SEM_MUTEX		"mutex"
#define SEM_RW			"readwrite"

// entry type
typedef struct entry {
	char key[MAX_KEY_SIZE];
	char *val; // we don't want to have a maximum-value size
	struct entry *next;
} store_entry;

// why not double-linked list? because the first one is created in
// shared memory, and there should be a theorical limit of number of dbs
typedef struct db {
	char name[MAX_DB_SIZE];
	struct db *next;
	struct entry *ent;
} store_db;

// for extracting info from shared memory
typedef struct info {
	pid_t pid;
	struct db *dbs;
} store_info;

// for get and set operations
struct entry_inf {
	char *key; // key/path to the name for the entry
	char *value; // key/path to the name for the entry
	struct entry *entry;
	char *db_name;
	struct db *dbs;
	int error;
};

void *store_data(size_t bytes);
void free_data(void *p);
void print_error_case(int error);
void print_error(char *msg);
int max(int x, int y);
int min(int x, int y);

#endif
