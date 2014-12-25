/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#include <stdio.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h> // fork
#include <fcntl.h> // O_CREAT, ...
#include "common.h"
#include "store.h"
#include "memory.h"
#include "disk.h"
#include "database.h"

int store_init()
{
	int error = 0;
	int shmid;
	store_db *store_dbs;
	key_t shm_key = ftok(".", KEY_ID);

	DEBUG_PRINT("notice: database starting\n");

	// create our root db
	shmid = shmget(shm_key, sizeof(struct db), IPC_CREAT | IPC_EXCL | 0660);

	// we shouldn't be able to initialize memory when it has already been done
	if(-1 != shmid)
	{
		// create additional semaphores and initiate our store db
		memory_init();
		
		// store_db.name = ...
		if((store_db *) -1 == (store_dbs = shmat(shmid, NULL, 0)))
		{
			error = ERR_STORE_SHMAT;
		}
		else
		{
			// our root db always has an empty name
			strcpy(store_dbs->name, "");
			store_dbs->next = NULL;

			// *** SYNC WITH FILESYSTEM ***

			// here we should ideally keep a daemon running and
			// check that our memory-values are the same as our
			// disk values
		}
	}
	else
	{
		error = ERR_STORE_SHMCREATE;
	}

	return error;
}

int store_set(char key[], char value[], int num_dbs, char *dbs[])
{
	int error = 0;
	int i = 0;
	pid_t pid;
	int therr = 0;
	store_entry **entries = NULL;
	pthread_t *thids = NULL;
	struct entry_inf *inf = NULL;
	FILE **fids = NULL;
	int shmid = 0;
	store_db *store_dbs = NULL;
	key_t shm_key = ftok(".", KEY_ID);

	DEBUG_PRINT("notice: supplied key \"%s\", value \"%s\", num_dbs %d\n",
	            key, value, num_dbs);

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif

	// the parent process modifies the memory so we can print our results
	// faster and let the other process continue writing to disk
	if((pid = fork()) > 0)
	{
		// get our memory location (our first root database with root values)
		// note we only need to do this if we are going to get-set via memory
		if((shmid = shmget(shm_key, sizeof(struct db), 0660)) == -1)
		{
			error = ERR_STORE_SHMLOAD;
		}
		else
		{
			if((store_db *) -1 == (store_dbs = shmat(shmid, NULL, 0)))
			{
				error = ERR_STORE_SHMAT;
			}

			#ifdef __DEBUG__
			else
			{
				print_existing_databases(store_dbs);
			}
			#endif
		}

		if(error == ERR_NONE)
		{
			thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t));
			entries = (store_entry **) calloc(num_dbs, sizeof(store_entry *));
			inf = (struct entry_inf *) calloc(num_dbs, sizeof(struct entry_inf));
	
			if(thids != NULL && entries != NULL && inf != NULL)
			{
				DEBUG_PRINT("alloc ok \n");

				// parent - alter the database in memory
				for(; i < num_dbs && ! therr && error == ERR_NONE; i++)
				{
				DEBUG_PRINT("for#%d \n", i);
					// create the entry information for setting
					inf[i].key = key;
					inf[i].value = value;
					inf[i].entry = entries[i];
					inf[i].db_name = dbs[i];
					inf[i].dbs = store_dbs;
					inf[i].error = 0;
				DEBUG_PRINT("values set, creating thread \n");
					
					// create our thread
					therr = pthread_create(&thids[i], NULL, memory_set,
					                       &inf[i]);
				DEBUG_PRINT("thread created \n");
	
					DEBUG_PRINT("notice: [parent] thread#%d %d (\"%s\" => \
\"%s\") to insert in db \"%s\"\n",
					            i, (int) thids[i], key, value, inf[i].db_name);

					if(therr != ERR_NONE) 
					{
						error = ERR_THR;
					}
				}
			}

			// set the number of iterations that went correctly
			num_dbs = i;
			therr = 0;
			
			// now, end our threads
			for(i = 0; i < num_dbs && therr == ERR_NONE && error == ERR_NONE; i++)
			{
				DEBUG_PRINT("notice: [parent] ending thread#parent-%d %d...\n",
				            i, (int) thids[i]);
				
				therr = pthread_join(thids[i], (void **) &error);

				if(therr != 0)
				{
					error = ERR_THRJOIN;
				}
				else
				{
					error = inf[i].error;
					DEBUG_PRINT("notice: [parent] ended thread#parent-%d %d\
returned value %d\n",
				                i, (int) thids[i], inf[i].error);
				}
			}
		}

		// unmap our shared memory
		if(error != ERR_STORE_SHMLOAD && -1 == shmdt(store_dbs))
		{
			error = ERR_STORE_SHMDT;
		}

		free(inf);
		free(thids);
		free(entries);
	}
	else if(pid == 0)
	{
		// create our thread entries and store entries
		inf = (struct entry_inf *) calloc(num_dbs, sizeof(struct entry_inf));
		thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t));
		entries = (store_entry **) calloc(num_dbs, sizeof(store_entry *));
		fids = (FILE **) calloc(num_dbs, sizeof(FILE *));
	

		/* if(fids != NULL && entries != NULL && thids != NULL)
		{
			// parent - alter the database in disk
			for(i = 0; i < num_dbs && ! therr && error == ERR_NONE; i++)
			{
				// fopen();

				strcpy(entries[i].key, key);
				strcpy(entries[i].db, dbs[i]);

				// copy the value
				val_len = (size_t) min((int) strlen(value), MAX_VAL_SIZE);
				entries[i].val = (char *) calloc(val_len, sizeof(char));
					
				if(entries[i].val != NULL)
				{
					strncpy(entries[i].val, value, val_len);

					// create our thread
					therr = pthread_create(&thids[i], NULL, disk_set, &entries[i]);
				
					DEBUG_PRINT("notice: [child] thread#%d %d (\"%s\" => \
\"%s\") to insert in db \"%s\"\n",
					            i, (int) thids[i],
								entries[i].key, entries[i].val, entries[i].db);
	
			
					if(therr != 0)
					{
						error = ERR_THR;
					}
				}
				else
				{
					error = ERR_ALLOC;
				}
			}

			// set the number of iterations
			num_dbs = i;
			therr = 0;

			// now, end our threads
			for(i = 0; i < num_dbs && ! therr && error == ERR_NONE; i++)
			{
				DEBUG_PRINT("notice: [child] ending thread#child-%d %d...\n",
				            i, (int) thids[i]);
				
				therr = pthread_join(thids[i], NULL);

				if(therr != 0)
				{
					error = ERR_THRJOIN;
				}
				else
				{
					DEBUG_PRINT("notice: [child] ended thread#child-%d %d\n",
					            i, (int) thids[i]);
				}

				// fclose();
			}
		}
		else
		{
			error = ERR_ALLOC;
		} */

		free(inf);
		free(thids);
		free(entries);
		free(fids);
	}
	else
	{
		error = ERR_FORK;
	}

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	int error = 0;
	int i;
	int therr = 0;
	store_db *store_dbs = NULL;
	store_entry **entries = NULL;
	pthread_t *thids = NULL;
	struct entry_inf *inf = NULL;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);
	int *th_return_err = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", num_dbs %d\n",
	            key, num_dbs);

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif

	// get our memory location (our first root database with root values)
	// note we only need to do this if we are going to get-set via memory
	if((shmid = shmget(shm_key, sizeof(struct db), 0660)) == -1)
	{
		error = ERR_STORE_SHMLOAD;
	}
	else
	{
		if((store_db *) -1 == (store_dbs = shmat(shmid, NULL, 0)))
		{
			error = ERR_STORE_SHMAT;
		}

		#ifdef __DEBUG__
		else
		{
			print_existing_databases(store_dbs);
		}
		#endif
	}

	if(error == ERR_NONE)
	{
		// create our thread entries
		thids = (pthread_t *) calloc(num_dbs,
		                                 sizeof(pthread_t));
		entries = (store_entry **) calloc(num_dbs, sizeof(store_entry *));
		inf = (struct entry_inf *) calloc(num_dbs, sizeof(struct entry_inf));

		if(thids != NULL && entries != NULL && inf != NULL)
		{
			for(i = 0; i < num_dbs && ! therr && error == ERR_NONE; i++)
			{
				// create our entry information
				inf[i].key = key;
				inf[i].entry = entries[i];
				inf[i].db_name = dbs[i];
				inf[i].dbs = store_dbs;
				inf[i].error = error;

				// create our thread
				therr = pthread_create(&thids[i], NULL, memory_get,
				                       &inf[i]);
	
				DEBUG_PRINT("notice: thread#%d %d (key \"%s\" \
 to search in db \"%s\"\n",
				            i, (int) thids[i], key, inf[i].db_name);

				if(therr != 0)
				{
					error = ERR_THR;
				}
			}
	
			// set our number of iterations that went correctly
			num_dbs = i;

			// end our threads
			for(i = 0; i < num_dbs && error == ERR_NONE; i++)
			{
				DEBUG_PRINT("notice: ending thread#%d %d...\n",
				            i, (int) thids[i]);
	
				therr = pthread_join(thids[i], (void **) &th_return_err);
	
				if(therr != 0)
				{
					DEBUG_PRINT("thread ended, error %d\n", therr);
					error = ERR_THRJOIN;
				}
				else
				{
					error = inf[i].error;
					DEBUG_PRINT("notice: ended thread#%d %d, \
returned %d\n",
					            i, (int) thids[i], error);
				}
			}

			// unmap our shared memory
			if(-1 == shmdt(store_dbs))
			{
				error = ERR_STORE_SHMDT;
			}
		}
		else
		{
			error = ERR_ALLOC;
		}
	}

	free(thids);
	free(inf);
	free(entries);

	return error;
}

int store_halt()
{
	int error = 0;
	int shmid;
	store_db *store_dbs = NULL;
	key_t shm_key = ftok(".", KEY_ID);

	DEBUG_PRINT("notice: database preparing to shut down\n");

	// get our memory location (our first root database with root values)
	// note we only need to do this if we are going to get-set via memory
	if((shmid = shmget(shm_key, sizeof(struct db), 0660)) == -1)
	{
		error = ERR_STORE_SHMLOAD;
	}
	else
	{
		if((store_db *) -1 == (store_dbs = shmat(shmid, NULL, 0)))
		{
			error = ERR_STORE_SHMAT;
		}
	}
	
	if(-1 == shmctl(shmid, IPC_RMID, NULL))
	{
		error = ERR_STORE_SHMCTL;
	}

	// free our memory
	if(error == ERR_NONE)
	{
		printf("freeing memory...\n");

		// sem_wait()
		// free entries[i].val, entries and dbs
		// -> call mem_clean() ???
		// sem_post()

		// close our semaphores
		error = memory_clear();
	}

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

void print_existing_databases(store_db *store_dbs)
{
	int i = 0;

    printf("\nlist of databases:\n------------------\n");
    while(store_dbs != NULL)
    {
		if(store_dbs->name != NULL)
		{
			printf("database#%d: %s\n", i, store_dbs->name);
		}
		else
		{
			printf("database#%d (unnamed)\n", i);
		}
		i++;
		store_dbs = store_dbs->next;
    }
    printf("-----------------\n\n");

}
#endif
