/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#include <stdio.h>
#include <pthread.h>
#include "common.h"
#include "disk.h"

void *disk_set(void *ent)
{
	printf("---%p---\n", ent);
	int error = 0;

	/*DEBUG_PRINT("notice: [parent, disk] setting in db \"%s\" key \"%s\", value \
\"%s\"\n",
	            (*((store_entry *) ent)->db)->name,
				((store_entry *) ent)->key,
				((store_entry *) ent)->val);*/

	pthread_exit(&error);
}

void *disk_get(void *ent)
{
	printf("---%p---\n", ent);
	int error = 0;

	/*DEBUG_PRINT("notice: [parent, disk] getting from db \"%s\" key \"%s\"\n",
	            (*((store_entry *) ent)->db)->name,
				((store_entry *) ent)->key);

	// set the value
	strcpy(((store_entry *) ent)->val, "TEST");

	DEBUG_PRINT("notice: [parent, disk] got value \"%s\" for key \"%s\" in db \
\"%s\"\n",
	            ((store_entry *) ent)->val,
				((store_entry *) ent)->key,
				(*((store_entry *) ent)->db)->name);*/

	pthread_exit(&error);
}
