/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#include <stdio.h>
#include <unistd.h> // fork
#include "common.h"
#include "store.h"

int store_init()
{
	int error = 0;

	DEBUG_PRINT("notice: database starting\n");

	return error;
}

int store_set(char key[], char value[], int num_dbs, char *dbs[])
{
	int error = 0;
	int i;
	pid_t pid;

	DEBUG_PRINT("notice: supplied key \"%s\", value \"%s\", num_dbs %d\n",
	            key, value, num_dbs);

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif

	pid = fork();
	if(pid > 0)
	{
		// parent - alter the database in disk
		for(i = 0; i < num_dbs; i++)
		{
			// pthread_create()
			DEBUG_PRINT("notice: [parent] create thread %d to insert in db \"%s\"\n",
			            0, dbs[i]);
		}
	}
	else if(pid == 0)
	{
		// children - alter the database in memory
		for(i = 0; i < num_dbs; i++)
		{
			// pthread_create()
			DEBUG_PRINT("notice: [child] create thread %d to insert in db \"%s\"\n",
			            0, dbs[i]);
		}
	}
	else
	{
		error = 2;
	}

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	int error = 0;
	int i;

	DEBUG_PRINT("notice: supplied key \"%s\", num_dbs %d\n",
	            key, num_dbs);

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif
	
	for(i = 0; i < num_dbs; i++)
	{
		// pthread_create()
		DEBUG_PRINT("notice: create thread %d to search in db \"%s\"\n", 0, dbs[i]);
	}

	return error;
}

int store_halt()
{
	int error = 0;

	DEBUG_PRINT("notice: database shutting down\n");

	return error;
}

#ifdef __DEBUG__
void print_databases(int num_dbs, char *dbs[])
{
	int i;

    printf("\nlist of databases:\n------------------\n");
    for(i = 0; i < num_dbs; i++)
    {
        printf("database#%d: %s\n", i, dbs[i]);
    }
    printf("-----------------\n\n");
}
#endif
