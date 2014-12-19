/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // fork
#include "common.h"
#include "store.h"
#include "memory.h"
#include "disk.h"

int store_init()
{
	int error = 0;

	DEBUG_PRINT("notice: database starting\n");

	memory_init();

	// here we should ideally keep our daemon running and
	// check that our memory-values are the same as our
	// disk values

	return error;
}

int store_set(char key[], char value[], int num_dbs, char *dbs[])
{
	int error = 0;
	int i;
	pid_t pid;
	int therr = 0;
	store_entry *entries = NULL;
	pthread_t *thids = NULL;
	FILE **fids = NULL;
	
	DEBUG_PRINT("notice: supplied key \"%s\", value \"%s\", num_dbs %d\n",
	            key, value, num_dbs);

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif

	// the parent process modifies the memory so we can print our results
	// faster and let the other process continue writing to disk
	pid = fork();
	if(pid > 0)
	{
		// create our thread entries and store entries
		thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t));
		entries = (store_entry *) calloc(num_dbs, sizeof(store_entry));

		if(thids != NULL && entries != NULL)
		{
			// children - alter the database in memory
			for(i = 0; i < num_dbs && ! therr; i++)
			{
				// set up our entry
				strcpy(entries[i].key, key);
				strcpy(entries[i].val, value);
				strcpy(entries[i].db, dbs[i]);
				DEBUG_PRINT("notice: store_entry variable: key=\"%s\" \
value=\"%s\" db=\"%s\"\n",
			                entries[i].key, entries[i].val, entries[i].db);
				
				DEBUG_PRINT("notice: [child] thread#%d %d to insert in db \"%s\"\n",
				            i, (int) thids[i], dbs[i]);
				
				// create our thread
				therr = pthread_create(&thids[i], NULL, memory_set, &entries[i]);
				
				DEBUG_PRINT("notice: [child] thread#%d %d to insert in db \"%s\"\n",
				            i, (int) thids[i], dbs[i]);

				if(therr != 0)
				{
					DEBUG_PRINT("error: [child] pthread_create for thread#%d %d \
returned non-zero exit code %d\n",
					            i, (int) thids[i], therr);
					error = 20;
				}
			}

			// set the number of iterations that went correctly
			num_dbs = i;
			therr = 0;
			
			// now, end our threads
			for(i = 0; i < num_dbs && ! therr; i++)
			{
				DEBUG_PRINT("notice: [child] ending thread#child-%d %d...\n",
				            i, (int) thids[i]);
				
				therr = pthread_join(thids[i], NULL);

				if(therr != 0)
				{
					DEBUG_PRINT("error: [child] pthread_join for thread#%d %d \
returned non-zero exit code %d\n",
					            i, (int) thids[i], therr);
					error = 20;
				}
				
				DEBUG_PRINT("notice: [child] ended thread#child-%d %d\n",
				            i, (int) thids[i]);
			}
		}
		else
		{
			DEBUG_PRINT("error: calloc returned NULL\n");
			error = 5;
		}

		if(thids != NULL)
		{
			free(thids);
		}

		if(entries != NULL)
		{
			free(entries);
		}
	}
	else if(pid == 0)
	{
		// create our thread entries and store entries
		thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t));
		entries = (store_entry *) calloc(num_dbs, sizeof(store_entry));
		fids = (FILE **) calloc(num_dbs, sizeof(FILE *));

		if(fids != NULL && entries != NULL && thids != NULL)
		{
			// parent - alter the database in disk
			for(i = 0; i < num_dbs && ! therr; i++)
			{
				// fopen();

				// set up our entry
				strcpy(entries[i].key, key);
				strcpy(entries[i].val, value);
				strcpy(entries[i].db, dbs[i]);
				DEBUG_PRINT("notice: store_entry variable: key=\"%s\" \
value=\"%s\" db=\"%s\"\n",
			                entries[i].key, entries[i].val, entries[i].db);
	
				DEBUG_PRINT("notice: [parent] thread#%d %d to insert in db \
\"%s\"\n",
				            i, (int) thids[i], dbs[i]);
				
				// create our thread
				therr = pthread_create(&thids[i], NULL, disk_set, &entries[i]);
				
				if(therr != 0)
				{
					DEBUG_PRINT("error: [parent] pthread_create for thread#%d %d \
returned non-zero exit code %d\n",
					            i, (int) thids[i], therr);
					error = 20;
				}
			}

			// set the number of iterations
			num_dbs = i;
			therr = 0;

			// now, end our threads
			for(i = 0; i < num_dbs && ! therr; i++)
			{
				DEBUG_PRINT("notice: [parent] ending thread#parent-%d %d...\n",
				            i, (int) thids[i]);
				
				therr = pthread_join(thids[i], NULL);

				if(therr != 0)
				{
					DEBUG_PRINT("error: [parent] pthread_join for thread#%d %d \
returned non-zero exit code %d\n",
					            i, (int) thids[i], therr);
					error = 20;
				}

				DEBUG_PRINT("notice: [parent] ended thread#parent-%d %d\n",
				            i, (int) thids[i]);

				// fclose();
			}
		}
		else
		{
			DEBUG_PRINT("error: calloc returned NULL\n");
			error = 5;
		}

		if(thids != NULL)
		{
			free(thids);
		}

		if(entries != NULL)
		{
			free(entries);
		}

		if(fids != NULL)
		{
			free(fids);
		}
	}
	else
	{
		DEBUG_PRINT("error: fork returned a number less than zero\n");
		error = 2;
	}

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	int error = 0;
	int i;
	char value[MAX_VAL_SIZE] = "";
	int therr = 0;
	store_entry *entries = NULL;
	pthread_t *thids = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", num_dbs %d\n",
	            key, num_dbs);

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif
	
	// create our thread entries
	thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t));
	entries = (store_entry *) calloc(num_dbs, sizeof(store_entry));

	if(thids != NULL && entries != NULL)
	{
		for(i = 0; i < num_dbs; i++)
		{
			strcpy(entries[i].key, key);
			strcpy(entries[i].val, value);
			strcpy(entries[i].db, dbs[i]);
			DEBUG_PRINT("notice: store_entry variable: key=\"%s\" value=\"%s\" \
db=\"%s\"\n",
			            entries[i].key, entries[i].val, entries[i].db);

			DEBUG_PRINT("notice: [child] thread#%d %d to search in db \"%s\"\n",
			            i, (int) thids[i], dbs[i]);

			// create our thread
			therr = pthread_create(&thids[i], NULL, memory_get, &entries[i]);

			if(therr != 0)
			{
				DEBUG_PRINT("error: [child] pthread_create for thread#%d %d \
returned non-zero exit code %d\n",
				            i, (int) thids[i], therr);
				error = 20;
			}
		}
	
		// set our number of iterations that went correctly
		num_dbs = i;

		// end our threads
		for(i = 0; i < num_dbs; i++)
		{
			DEBUG_PRINT("notice: [child] ending thread#%d %d...\n",
			            i, (int) thids[i]);

			therr = pthread_join(thids[i], NULL);

			if(therr != 0)
			{
				DEBUG_PRINT("error: [child] pthread_join for thread#%d %d \
returned non-zero exit code %d\n",
				            i, (int) thids[i], therr);
				error = 20;
			}
				
			DEBUG_PRINT("notice: [child] ended thread#%d %d\n",
			            i, (int) thids[i]);
		}
	}
	else
	{
		DEBUG_PRINT("error: calloc returned NULL\n");
		error = 5;
	}

	if(thids != NULL)
	{
		free(thids);
	}

	if(entries != NULL)
	{
		free(entries);
	}

	return error;
}

int store_halt()
{
	int error = 0;

	DEBUG_PRINT("notice: database preparing to shut down\n");

	memory_clear();

	// free our memory
	// here we should ideally check that our disk-values are
	// the same as our memory-values

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
