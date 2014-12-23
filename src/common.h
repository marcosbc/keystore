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
#define ERR_CALLOC 5
#define ERR_CALLOC_MSG "calloc() returned NULL"
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
#define ERR_THRJOIN_MSG ""

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
	char db[MAX_DB_SIZE];
	struct entry *next;
} store_entry;

// dictionary type
// why tree? -> first_dic:second_dic:third_dic:entry
typedef struct collection {
	char name[MAX_DICT_SIZE];
	struct entry *ent;
	struct collection *next; // in chronological order
	struct collection *children;
	struct collection *brother;
} store_collection;

// why not double-linked list? because the first one is created in
// shared memory, and there should be a theorical limit of number of dbs
typedef struct db {
	char name[MAX_DB_SIZE];
	struct collection *col;
	struct db *next;
} store_db;

int extract_collection(char collection[MAX_KEY_SIZE], char key[MAX_KEY_SIZE]);
void print_error_case(int error);
void print_error(char *msg);
int max(int x, int y);
int min(int x, int y);

#endif
