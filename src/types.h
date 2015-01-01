/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_STORE__
#define __KEYSTORE_STORE__

#include <sys/un.h>

#define MAX_DB_SIZE       16
#define MAX_KEY_SIZE      32
#define MAX_VAL_SIZE      0
#define STORE_ACK         "ok"
#define STORE_SOCKET_PATH "keystore_server.sock"
#define STORE_MODE_SET    's'
#define STORE_MODE_GET    'g'
#define STORE_MODE_STOP   'x'

// entry type
typedef struct entry {
	char key[MAX_KEY_SIZE];
	char *val; // we don't want to have a maximum-value size
	struct entry *next;
	struct entry *brother; // to iterate entries in dbs
} store_entry;

// why not double-linked list? because the first one is created in
// shared memory, and there should be a theorical limit of number of dbs
typedef struct db {
	char name[MAX_DB_SIZE];
	struct db *next;
	struct entry *ent;
} store_db;

// for get and set operations
struct entry_inf {
	char key[MAX_KEY_SIZE]; // key/path to the name for the entry
	char *value; // key/path to the name for the entry
	struct entry *entry;
	char *db_name;
	struct db **dbs;
	int error;
};

#endif
