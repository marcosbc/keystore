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

#define MAX_PATH_SIZE 1024
#define MAX_DB_SIZE 16
#define MAX_KEY_SIZE 16
#define MAX_VAL_SIZE 512

typedef struct entry {
	char key[MAX_KEY_SIZE];
	char val[MAX_VAL_SIZE];
	char db[MAX_DB_SIZE];
	struct entry *next;
} store_entry;

#endif
