/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#include <stdio.h>
#include <pthread.h>
#include "common.h"
#include "memory.h"

void *memory_set(void *ent)
{
	int error = 0;

	DEBUG_PRINT("notice: [child, memory] setting in db \"%s\" key \"%s\", value \
\"%s\"\n",
	            ((store_entry *) ent)->db,
				((store_entry *) ent)->key,
				((store_entry *) ent)->val);

	pthread_exit(&error);
}

void *memory_get(void *ent)
{
	int error = 0;

	DEBUG_PRINT("notice: [child, memory] getting from db \"%s\" key \"%s\"\n",
	            ((store_entry *) ent)->db,
				((store_entry *) ent)->key);

	// set the value
	strcpy(((store_entry *) ent)->val, "TEST");

	DEBUG_PRINT("notice: [child, memory] got value \"%s\" for key \"%s\" in db \
\"%s\"\n",
	            ((store_entry *) ent)->val,
				((store_entry *) ent)->key,
				((store_entry *) ent)->db);

	pthread_exit(&error);
}
