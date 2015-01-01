/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> // O_CREAT, ...
#include "common.h"
#include "memory.h"
#include "database.h"

int shmid = -1;
sem_t *sem_mutex = NULL;
sem_t *sem_rw = NULL;

int memory_init()
{
	int ok = 1;
	
	DEBUG_PRINT("notice: creating semaphores for memory operations\n");

	// if we don't unlink them, they give problems
	
	// create semaphores
	sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1); // binary semaphore
	sem_rw = sem_open(SEM_RW, O_CREAT, 0666, MAX_READERS_AT_ONCE);

	if(sem_mutex == (sem_t *) -1 || sem_rw == (sem_t *) -1)
	{
		if(sem_mutex == (sem_t *) -1)
		{
			print_error("couldn't create semaphore \"%s\"", SEM_MUTEX);
		}
		if(sem_rw == (sem_t *) -1)
		{
			print_error("couldn't create semaphore \"%s\"", SEM_RW);
		}
		print_perror("sem_open");
		ok = 0;
	}

	return ok;
}

void *memory_set(void *info)
{
	DEBUG_PRINT("in memory_set\n");
	int val_len = 0;
	store_entry *entry = NULL;
	store_db *db = NULL;

	// extract information from our info variable
	char *key = ((struct entry_inf *) info)->key;
	char *value = ((struct entry_inf *) info)->value;
	char *db_name = ((struct entry_inf *) info)->db_name;
	store_db **dbs = ((struct entry_inf *) info)->dbs;
	int *error = &(((struct entry_inf *) info)->error);

	DEBUG_PRINT("got values: key=%s value=%s db_name=%s dbs=%p error=%d\n",
	            key, value, db_name, *dbs, *error);

	if((sem_t *) -1 == sem_rw || (sem_t *) -1 == sem_mutex)
	{
		*error = ERR_MEM_SEMOPEN;
		if(sem_mutex == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"",
			        SEM_MUTEX);
		}
	
		if(sem_rw == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"", SEM_RW);
		}
	}
	else
	{
		// we shouldn't write while reading/writing
		memory_write_lock();
	
		DEBUG_PRINT("finding db\n");
		// locate our db and find our entry
		db = locate_db(db_name, *dbs);

		DEBUG_PRINT("db found?\n");
		// create our db if it doesn't exist
		if(db == NULL)
		{
			DEBUG_PRINT("*** creating db with name \"%s\", dbs=%p\n",
			            db_name, dbs);
			db = create_db(db_name, dbs);
			DEBUG_PRINT("created db %p\n", db);
		}

		// error allocating data while creating the db?
		if(db == NULL)
		{
			DEBUG_PRINT("error allocating for db \n");
			*error = ERR_ALLOC;
		}
		else
		{
			DEBUG_PRINT("finding entry\n");


			#ifdef __DEBUG__
			DEBUG_PRINT("\n\nstore tree BEFORE");
			print_store_tree(&db);
			#endif


			entry = locate_entry(key, db);
			DEBUG_PRINT("entry found?\n");


			#ifdef __DEBUG__
			DEBUG_PRINT("\n\nstore tree AFTER");
			print_store_tree(&db);
			#endif


			#ifdef __DEBUG__
			DEBUG_PRINT("\n\nstore tree AFTER 2");
			print_store_tree(&db);
			#endif



			// did we find an entry?
			if(entry != NULL)
			{
				// yes, so free the previous one
				DEBUG_PRINT("yes, free val %p\n", entry->val);
				free(entry->val);
				entry->val = NULL;
			}
			// if we didn't, create a new one
			else
			{
				DEBUG_PRINT("no, creat\n");



				entry = create_entry(key, &db);
				DEBUG_PRINT("created\n");


				if(entry == NULL)
				{
					*error = ERR_ALLOC;
				}
			}

			if(*error == ERR_NONE)
			{
				// if we have set a value limit, apply it
				if(MAX_VAL_SIZE <= 0)
				{
					val_len = strlen(value) + 1;
				}
				else
				{
					val_len = (size_t) min((int) strlen(value) + 1, MAX_VAL_SIZE);
				}
				DEBUG_PRINT("reserving val %s, len %d\n", value, val_len);
				entry->val = (char *) calloc(val_len, sizeof(char));
					
				if(entry->val == NULL)
				{
					*error = ERR_ALLOC;
				}
				DEBUG_PRINT("have entry now, adding value (MAX_LEN=%d)\n",
				            MAX_VAL_SIZE);
				
				strncpy(entry->val, value, val_len);
				entry->val[val_len - 1] = '\0';

				DEBUG_PRINT("notice: [child, memory] setting in db \"%s\" key \
\"%s\", value \"%s\" and val_len \"%d\" is DONE\n",
				            db_name, entry->key, entry->val, val_len);

				// save the entry to our info variable (as output)
				((struct entry_inf *) info)->entry = entry;

				#ifdef __DEBUG__
				print_store_tree(dbs);

				if(entry != NULL)
				{
					DEBUG_PRINT("\n\nhas entry: %p with %s=%s\n", entry,
					            entry->key, entry->val);
				}
				else
				{
					DEBUG_PRINT("\n\nwe *DONT* have an entry, NULL: %p\n", entry);
				}
				#endif
			}
		}

		// done!
		memory_write_unlock();
	}
	
	DEBUG_PRINT("returning thread error %d\n", *error);
	pthread_exit(NULL);
}

void *memory_get(void *info)
{
	DEBUG_PRINT("in memory_get\n");
	store_entry *ent = NULL;

	// extract information from our info variable
	char *key = ((struct entry_inf *) info)->key;
	char *value = NULL;
	char *db_name = ((struct entry_inf *) info)->db_name;
	store_db *dbs = *(((struct entry_inf *) info)->dbs);
	store_db *db = NULL;
	int *error = &(((struct entry_inf *) info)->error);

	if(sem_rw == (sem_t *) -1 || sem_mutex == (sem_t *) -1)
	{
		*error = ERR_MEM_SEMOPEN;
		if(sem_mutex == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"", SEM_MUTEX);
		}
	
		if(sem_rw == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"", SEM_RW);
		}
	}
	else
	{
		// we have a limit of max readers at once
		memory_read_lock();

		DEBUG_PRINT("locating db %s in dbs %p\n", db_name, dbs);

		// locate our db and find our entry
		if(NULL == (db = locate_db(db_name, dbs)))
		{
			*error = ERR_DB;
		}
		else if(NULL == (ent = locate_entry(key, db)))
		{
			*error = ERR_ENTRY;
		}
		else if(NULL == (value = (char *) calloc((strlen(ent->val) + 1),
		                                         sizeof(char))))
		{
			*error = ERR_ALLOC;
		}
		else
		{
			DEBUG_PRINT("notice: [child, memory] getting from db \"%s\" key \
\"%s\"\n",
			            db->name,
			    		((store_entry *) ent)->key);

			// get the value from the db
			// write semaphore not needed because the entry_info is not shared
			strcpy(value, ent->val);

			DEBUG_PRINT("notice: [child, memory] got value \"%s\" for key \
\"%s\" in db \"%s\"\n",
			            value, key, db_name);
			
			// save the entry and value to our info variable (as output)
			((struct entry_inf *) info)->entry = ent;
			((struct entry_inf *) info)->value = value;

			#ifdef __DEBUG__
			print_store_tree(&dbs);

			if(ent != NULL)
			{
				DEBUG_PRINT("\n\nhas entry: %p with %s=%s\n", ent,
				            ent->key, ent->val);
			}
			else
			{
				DEBUG_PRINT("\n\nwe *DONT* have an entry, NULL: %p\n", ent);
			}
			#endif
		}

		// reading done!
		memory_read_unlock();
	}
	
	DEBUG_PRINT("returning thread error %d\n", *error);
	pthread_exit(NULL);
}

int memory_clear(store_db **dbs)
{
	int error = 0;
	
	DEBUG_PRINT("notice: unlinking semaphores\n");

	if(! free_tree(dbs, &error))
	{
		print_error("memory couldn't be freed");
	}
	
	if(sem_rw != (sem_t *) -1)
	{
		sem_close(sem_rw);
	}
	
	if(sem_mutex != (sem_t *) -1)
	{
		sem_close(sem_mutex);
	}

	if(-1 == sem_unlink(SEM_MUTEX))
	{
		error = ERR_MEM_SEMUNLINK;
		print_error("couldn't unlink semaphore \"%s\"", SEM_RW);
	}

	if(-1 == sem_unlink(SEM_RW))
	{
		error = ERR_MEM_SEMUNLINK;
		print_error("couldn't unlink semaphore \"%s\"", SEM_MUTEX);
	}

	return error;
}

int free_tree(store_db **dbs, int *error)
{
	int success = 0;
	store_db *prev_db = NULL;
	store_db *iterator = NULL;
	store_entry *entry = NULL;
	store_entry *prev = NULL;

	if(sem_rw == (sem_t *) -1 || sem_mutex == (sem_t *) -1)
	{
		*error = ERR_MEM_SEMOPEN;
		if(sem_mutex == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"",
			        SEM_MUTEX);
		}
	
		if(sem_rw == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"",
			        SEM_RW);
		}
		print_perror("sem_open");
	}
	else
	{
		memory_write_lock();
		iterator = *dbs;

		// see what our dbs contains now
		while(iterator != NULL)
		{
			DEBUG_PRINT("current db: %p, next: %p\n", iterator, iterator->next);
			entry = iterator->ent;
			while(entry != NULL)
			{
				// free it's value
				DEBUG_PRINT("free val %p\n", entry->val);
				free(entry->val);
				prev = entry;
				entry = entry->next;
			
				// go to the next one
				DEBUG_PRINT("free entry %p\n", prev);
				free(prev);
			}
	
			DEBUG_PRINT("current db: %p, next: %p\n", iterator, iterator->next);
			prev_db = iterator;
			iterator = iterator->next;
			DEBUG_PRINT("free db %p, next is %p\n", prev_db, iterator);
			free(prev_db);
			prev_db = NULL;
		}

		*dbs = NULL;
		memory_write_unlock();
		success = 1;
	}
	
	return success;
}



void memory_read_lock()
{
	DEBUG_PRINT("read: going for rw wait...\n");

	sem_wait(sem_rw);
	
	DEBUG_PRINT("read: done\n");
}

void memory_read_unlock()
{
	sem_post(sem_rw);
}

void memory_write_lock()
{
	int i;

	DEBUG_PRINT("write: going for mutex wait, rw=%p and mut=%p...\n",
	            sem_rw, sem_mutex);

	// in case we have multiple writers
	sem_wait(sem_mutex);
	
	DEBUG_PRINT("write: going for write wait...\n");
			
	for(i = 0; i < MAX_READERS_AT_ONCE; i++)
	{
		DEBUG_PRINT("write: iteration %d\n", i);
		// we have a maximum number of readers
		// but we want to be able to read simultaneously
		sem_wait(sem_rw);
	}

	DEBUG_PRINT("write: going for mutex post...\n");

	// we shouldn't have problems with multiple writers now
	sem_post(sem_mutex);
	
	DEBUG_PRINT("write: done\n");
}

void memory_write_unlock()
{
	int i;

	for(i = 0; i < MAX_READERS_AT_ONCE; i++)
	{
		// semaphore++
		sem_post(sem_rw);
	}
}
