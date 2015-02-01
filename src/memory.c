#include <pthread.h>
#include "common.h"
#include "daemon.h"
#include "types.h"
#include "sems.h"
#include "memory.h"
#include "database.h"

int sems_is_init = 0;
int shmid = -1;

int memory_init()
{
	sems_is_init = sems_init();

	return sems_is_init;
}

void *memory_set(void *info)
{
	DEBUG_PRINT("notice: memory_set\n");
	int val_len = 0;
	int val_differs = 1; // if the values are the same
	store_entry *entry = NULL;
	store_db *db = NULL;

	// extract information from our info variable
	char *key = ((struct entry_inf *) info)->key;
	char *value = ((struct entry_inf *) info)->value;
	char *db_name = ((struct entry_inf *) info)->db_name;
	store_db **dbs = ((struct entry_inf *) info)->dbs;
	int *error = &(((struct entry_inf *) info)->error);

	// we shouldn't write while reading/writing
	write_lock();
	
	// locate our db and find our entry
	db = locate_db(db_name, *dbs);

	// create our db if it doesn't exist
	if(db == NULL)
	{
		DEBUG_PRINT("notice: db \"%s\" %p not found, creating...\n", db_name, dbs);
		db = create_db(db_name, dbs);
	}

	// error allocating data while creating the db?
	if(db == NULL)
	{
		perror("create_db");
		*error = ERR_ALLOC;
	}
	// are we unsetting an entry?
	else if(value[0] == '\0')
	{
		// we don't wan't to give any error if entry was not found
		delete_entry(key, db);
	}
	else
	{
		entry = locate_entry(key, db);

		// no value size limit?
		if(MAX_VAL_SIZE <= 0)
		{
			val_len = strlen(value) + 1;
		}
		// if we have set a value limit, apply it
		else
		{
			val_len = (size_t) min((int) strlen(value) + 1, MAX_VAL_SIZE);
		}

		// did we find an entry?
		if(entry != NULL)
		{
			if(0 == strncmp(value, entry->val, val_len))
			{
				DEBUG_PRINT("notice: %s: values are equal\n", db_name);
				val_differs = 0;
			}
			else
			{
				// if the value has changed, free the previous one
				free(entry->val);
				entry->val = NULL;
			}
		}
		// if we didn't, create a new one
		else
		{
			entry = create_entry(key, db);

			if(entry == NULL)
			{
				print_perror("create_entry");
				*error = ERR_ALLOC;
			}
		}

		if(*error == ERR_NONE)
		{
			// reserve space for our value as long as it was not the same
			if(val_differs)
			{
				entry->val = (char *) malloc(val_len * sizeof(char));
				
				if(entry->val == NULL)
				{
					print_perror("malloc");
					*error = ERR_ALLOC;
				}
				else
				{
					strncpy(entry->val, value, val_len);
					entry->val[val_len - 1] = '\0';
				}
			}

			DEBUG_PRINT("notice: \"%s\": setting \"%s\"=\"%s\" (\"%d\"B) DONE\n",
			            db_name, entry->key, entry->val, val_len);

			// save the entry to our info variable (as output)
			((struct entry_inf *) info)->entry = entry;

			#ifdef __DEBUG__
			print_store_tree(dbs);
			#endif
		}
	}

	// done!
	write_unlock();
	
	DEBUG_PRINT("notice: memory_set returning thread error %d\n", *error);
	pthread_exit(NULL);
}

void *memory_get(void *info)
{
	DEBUG_PRINT("notice: memory_get\n");
	store_entry *ent = NULL;

	// extract information from our info variable
	char *key = ((struct entry_inf *) info)->key;
	char *value = NULL;
	char *db_name = ((struct entry_inf *) info)->db_name;
	store_db *dbs = *(((struct entry_inf *) info)->dbs);
	store_db *db = NULL;
	int *error = &(((struct entry_inf *) info)->error);

	// we have a limit of max readers at once
	read_lock();

	DEBUG_PRINT("notice: locating db %s in dbs %p\n", db_name, dbs);

	// locate our db and find our entry
	if(NULL == (db = locate_db(db_name, dbs)))
	{
		*error = ERR_DB;
	}
	else if(NULL == (ent = locate_entry(key, db)))
	{
		*error = ERR_ENTRY;
	}
	else if(NULL == (value = (char *) malloc((strlen(ent->val) + 1)
	                                         * sizeof(char))))
	{
		print_perror("malloc");
		*error = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("notice: getting from db \"%s\" key \"%s\"\n",
		            db->name, ((store_entry *) ent)->key);

		// get the value from the db
		// write semaphore not needed because the entry_info is not shared
		strcpy(value, ent->val);

		DEBUG_PRINT("notice: %s: got \"%s\"=\"%s\"\n", db_name, key, value);
		
		// save the entry and value to our info variable (as output)
		((struct entry_inf *) info)->entry = ent;
		((struct entry_inf *) info)->value = value;

		#ifdef __DEBUG__
		print_store_tree(&dbs);

		if(ent != NULL)
		{
			DEBUG_PRINT("\nnotice: %s: got entry %p \"%s\"=\"%s\"\n\n",
			            db_name, ent, ent->key, ent->val);
		}
		else
		{
			DEBUG_PRINT("\nnotice: %s: entry \"%s\" *NOT FOUND* \n\n",
			            db_name, key);
		}
		#endif
	}

	// reading done!
	read_unlock();
	
	DEBUG_PRINT("notice: memory_get returning thread error %d\n", *error);
	pthread_exit(NULL);
}

int memory_clear(store_db **dbs)
{
	int error = ERR_NONE;

	// first, clear databases and entries
	free_tree(dbs);
	
	DEBUG_PRINT("notice: unlinking semaphores\n");
	if(! sems_close())
	{
		error = ERR_SEMCLOSE;
		perror("sem_unlink");
	}

	if(-1 == sem_unlink(SEM_MUTEX))
	{
		error = ERR_SEMUNLINK;
		print_error("couldn't unlink semaphore \"%s\"", SEM_RW);
		perror("sem_unlink");
	}

	if(-1 == sem_unlink(SEM_RW))
	{
		error = ERR_SEMUNLINK;
		print_error("couldn't unlink semaphore \"%s\"", SEM_MUTEX);
		perror("sem_unlink");
	}

	return error;
}

void free_tree(store_db **dbs)
{
	DEBUG_PRINT("notice: freeing the tree of databases and entries\n");

	store_db *prev_db = NULL;
	store_db *iterator = NULL;
	store_entry *entry = NULL;
	store_entry *prev = NULL; // previous entry

	write_lock();
	iterator = *dbs;

	// see what our dbs contains now
	while(iterator != NULL)
	{
		entry = iterator->ent;
		while(entry != NULL)
		{
			// free it's value
			DEBUG_PRINT("notice: freeing val %p\n", entry->val);
			free(entry->val);
			prev = entry;
			entry = entry->next;
		
			// go to the next one
			DEBUG_PRINT("notice: freeing entry %p\n", prev);
			free(prev);
		}

		// jump to next element
		prev_db = iterator;
		iterator = iterator->next;

		DEBUG_PRINT("notice: freeing db: %p, next: %p\n", prev_db, iterator);

		// free previous element
		free(prev_db);
	}

	*dbs = NULL;
	write_unlock();
}
