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
	int error = 0;
	key_t shm_key = ftok(".", KEY_ID);

	// we can't write while reading/writing

	DEBUG_PRINT("notice: [child, memory] setting in db \"%s\" key \"%s\", value \
\"%s\"\n",
	            ((store_entry *) ent)->db,
				((store_entry *) ent)->key,
				((store_entry *) ent)->val);
	
	// set the value in the db


	pthread_exit(&error);
}

void *memory_get(void *ent)
{
	int error = 0;
	int shmid;
	sem_t *sem_mutex, *sem_rw;
	key_t shm_key = ftok(".", KEY_ID);

	// we should have a maximum number of readers

	DEBUG_PRINT("notice: [child, memory] getting from db \"%s\" key \"%s\"\n",
	            ((store_entry *) ent)->db,
				((store_entry *) ent)->key);

	// get the value from the db
	strcpy(((store_entry *) ent)->val, "TEST");

	// semaphore++

	DEBUG_PRINT("notice: [child, memory] got value \"%s\" for key \"%s\" in db \
\"%s\"\n",
	            ((store_entry *) ent)->val,
				((store_entry *) ent)->key,
				((store_entry *) ent)->db);

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
	
	shmid = shmget(shm_key, SHM_SIZE * sizeof(store_entry), 0660);
	
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
