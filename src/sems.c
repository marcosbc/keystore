#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h> // O_CREAT, ...
#include "common.h"
#include "sems.h"

sem_t *sem_mutex = NULL;
sem_t *sem_rw = NULL;

int sems_init()
{
	int ok = 1;
	
	DEBUG_PRINT("notice: creating semaphores for memory operations\n");

	// if we don't unlink them before, they will give problems if they exist
	sem_unlink(SEM_RW);
	sem_unlink(SEM_MUTEX);

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

int sems_open()
{
	int ok = 1;
	sem_rw = sem_open(SEM_RW, 0);
	sem_mutex = sem_open(SEM_MUTEX, 0);

	if((sem_t *) -1 == sem_rw || (sem_t *) -1 == sem_mutex)
	{
		if(sem_mutex == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"",
			        SEM_MUTEX);
		}
	
		if(sem_rw == (sem_t *) -1)
		{
			print_error("couldn't open semaphore \"%s\"", SEM_RW);
		}
		ok = 0;
	}

	return ok;
}

int sems_close()
{
	int ok = 1;

	DEBUG_PRINT("notice: closing semaphores\n");

	if(sem_rw != (sem_t *) -1)
	{
		if(-1 == sem_close(sem_rw))
		{
			ok = 0;
		}
	}
	
	if(sem_mutex != (sem_t *) -1)
	{
		if(-1 == sem_close(sem_mutex))
		{
			ok = 0;
		}
	}

	return ok;
}

int sems_clear()
{
	int error = ERR_NONE;
	
	DEBUG_PRINT("notice: unlinking semaphores\n");

	if(! sems_close())
	{
		error = ERR_MEM_SEMCLOSE;
		perror("sem_unlink");
	}

	if(-1 == sem_unlink(SEM_MUTEX))
	{
		error = ERR_MEM_SEMUNLINK;
		print_error("couldn't unlink semaphore \"%s\"", SEM_RW);
		perror("sem_unlink");
	}

	if(-1 == sem_unlink(SEM_RW))
	{
		error = ERR_MEM_SEMUNLINK;
		print_error("couldn't unlink semaphore \"%s\"", SEM_MUTEX);
		perror("sem_unlink");
	}

	return error;
}

void read_lock()
{
	DEBUG_PRINT("readlock: going for rw wait...\n");

	sem_wait(sem_rw);
	
	DEBUG_PRINT("readlock: done\n");
}

void read_unlock()
{
	DEBUG_PRINT("readunlock\n");

	sem_post(sem_rw);
}

void write_lock()
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

void write_unlock()
{
	DEBUG_PRINT("writeunlock\n");
	
	int i;

	for(i = 0; i < MAX_READERS_AT_ONCE; i++)
	{
		// semaphore++
		sem_post(sem_rw);
	}
}
