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

#define MAX_PATH_SIZE	1024
#define MAX_DB_SIZE		16
#define MAX_DICT_SIZE	16
#define MAX_KEY_SIZE	16
#define MAX_VAL_SIZE	512

// for shared memory RW (we want to be able to have multiple readers,
// but block them only if we are writing)
#define MAX_READERS_AT_ONCE 10
#define ONE             1

#define SEM_MUTEX		"mutex"
#define SEM_RW			"readwrite"

// entry type
// why double-linked list? because we modify each entry after write
// to have a more fair search
typedef struct entry {
	char key[MAX_KEY_SIZE];
	char val[MAX_VAL_SIZE];
	char db[MAX_DB_SIZE];
	struct entry *next;
	struct entry *prev;
} store_entry;

// dictionary type
// why double-linked list? because we modify each dictionary after write
// to have a more fair search
// why tree? -> first_dic:second_dic:third_dic:entry
typedef struct dictionary {
	char name[MAX_DICT_SIZE];
	struct entry *start;
	struct dictionary *next;
	struct dictionary *prev;
	struct dictionary *children;
} store_dictionary;

typedef struct db {
	char name[MAX_DB_SIZE];
	struct dictionary *dict;
} store_db;

#endif
