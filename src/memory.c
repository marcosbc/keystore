/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#include <stdio.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> // O_CREAT, ...
#include "common.h"
#include "memory.h"

// shared memory:
//   to get different databases
//   each database has a pointer to each collection
//   collections (app:settings) have pointer to elements
//   we want the elements (app:settings:setting)

int memory_init()
{
	int error = 0;
	int shmid;
	sem_t *sem_mutex, *sem_rw;
	key_t shm_key = ftok(".", KEY_ID);
	
	DEBUG_PRINT("notice: creating shared memory\n");

	// for this project, we're going to use shared memory
	// ideally we should use linked lists
	shmid = shmget(shm_key, SHM_SIZE * sizeof(store_entry),
	               IPC_CREAT | IPC_EXCL | 0660);

	if(shmid != -1)
	{
		// create semaphores
		sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0600, ONE); // binary semaphore
		sem_rw = sem_open(SEM_RW, O_CREAT, 0600, MAX_READERS_AT_ONCE);

		DEBUG_PRINT("notice: creating semaphores for memory operations\n");

		if(sem_mutex == (sem_t *) -1)
		{
			fprintf(stderr, "error: couldn't create semaphore \"%s\"\n", SEM_MUTEX);
			error = 16;
		}

		if(sem_rw == (sem_t *) -1)
		{
			fprintf(stderr, "error: couldn't create semaphore \"%s\"\n", SEM_RW);
			error = 16;
		}
	}
	else
	{
		error = 15;
		fprintf(stderr, "error: couldn't create shared memory\n");
	}

	return error;
}

void *memory_set(void *ent)
{
	int shmid;
	int error = 0;
	store_db *store_dbs;
	key_t shm_key = ftok(".", KEY_ID);
	sem_t *sem_rw = sem_open(SEM_RW, 0);
	sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

	// load the shared memory
	shmid = shmget(shm_key, SHM_SIZE * sizeof(store_entry), 0660);

	if(shmid != -1)
	{
		if((store_db *) -1 != (store_dbs = shmat(shmid, NULL, 0)))
		{
			// we shouldn't write while reading/writing
			memory_write_lock(sem_rw, sem_mutex);

			DEBUG_PRINT("notice: [child, memory] setting in db \"%s\" key \
\"%s\", value \"%s\"\n",
			            ((store_entry *) ent)->db,
						((store_entry *) ent)->key,
						((store_entry *) ent)->val);
	
			// set the value in the db
			
			// done!
			memory_write_unlock(sem_rw);
		}
		else
		{
			error = 18;
			fprintf(stderr, "error mapping shared memory segment\n");
		}
	}
	else
	{
		error = 15;
		fprintf(stderr, "error: couldn't open shared memory");
	}

	pthread_exit(&error);
}

void *memory_get(void *ent)
{
	int error = 0;
	int shmid;
	store_db *store_dbs;
	key_t shm_key = ftok(".", KEY_ID);
	sem_t *sem_rw = sem_open(SEM_RW, 0);
	sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

	shmid = shmget(shm_key, SHM_SIZE * sizeof(store_entry), 0660);

	if(shmid != -1 && sem_rw != (sem_t *) -1 && sem_mutex != (sem_t *) -1)
	{
		if((store_db *) -1 != (store_dbs = shmat(shmid, NULL, 0)))
		{
			// we have a limit of max readers at once
			memory_read_lock(sem_rw);

			DEBUG_PRINT("notice: [child, memory] getting from db \"%s\" key \
\"%s\"\n",
			            ((store_entry *) ent)->db,
			    		((store_entry *) ent)->key);

			// get the value from the db
			strcpy(((store_entry *) ent)->val, "TEST");

			// reading done!
			memory_read_unlock(sem_rw);

			DEBUG_PRINT("notice: [child, memory] got value \"%s\" for key \
\"%s\" in db \"%s\"\n",
			            ((store_entry *) ent)->val,
			    		((store_entry *) ent)->key,
			    		((store_entry *) ent)->db);
		}
		else
		{
			error = 18;
			fprintf(stderr, "error mapping shared memory segment\n");
		}
	}
	else
	{
		if(shmid == -1)
		{
			error = 15;
			fprintf(stderr, "error: couldn't open shared memory\n");
		}

		else
		{
			error = 18;
			if(sem_mutex == (sem_t *) -1)
			{
				fprintf(stderr, "error: couldn't open semaphore \"%s\"\n",
				        SEM_MUTEX);
			}
		
			if(sem_rw == (sem_t *) -1)
			{
				fprintf(stderr, "error: couldn't open semaphore \"%s\"\n",
				        SEM_RW);
			}
		}
	}

	pthread_exit(&error);
}

int memory_clear()
{
	int error = 0;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);
	
	DEBUG_PRINT("notice: unlinking semaphores\n");

	if(-1 == sem_unlink(SEM_MUTEX))
	{
		fprintf(stderr, "error: couldn't unlink semaphore \"%s\"\n", SEM_RW);
	}

	if(-1 == sem_unlink(SEM_RW))
	{
		fprintf(stderr, "error: couldn't unlink semaphore \"%s\"\n", SEM_MUTEX);
	}

	DEBUG_PRINT("notice: removing shared memory\n");
	
	shmid = shmget(shm_key, SHM_SIZE * sizeof(store_db), 0660);
	
	if(-1 != shmid)
	{
		if(-1 == shmctl(shmid, IPC_RMID, NULL))
		{
			fprintf(stderr, "error: the shared memory couldn't be removed\n");
			error = 17;
		}
	}
	else
	{
		fprintf(stderr, "error: the shared memory couldn't be opened\n");
		error = 15;
	}

	return error;
}

void memory_read_lock(sem_t *sem)
{
	sem_wait(sem);
}

void memory_read_unlock(sem_t *sem)
{
	sem_post(sem);
}

void memory_write_lock(sem_t *sem, sem_t *mutex)
{
	int i;

	// in case we have multiple writers
	sem_wait(mutex);
			
	// we want the writer to have equal chances to be able to
	// use our shared memory
	for(i = 0; i < MAX_READERS_AT_ONCE; i++)
	{
		// we have a maximum number of readers
		// but we want to be able to read simultaneously
		sem_wait(sem);
	}

	// we shouldn't have problems with multiple writers now
	sem_post(mutex);
}

void memory_write_unlock(sem_t *sem)
{
	int i;

	for(i = 0; i < MAX_READERS_AT_ONCE; i++)
	{
		// semaphore++
		sem_post(sem);
	}
}
