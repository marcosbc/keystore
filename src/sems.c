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

void read_lock()
{
	DEBUG_PRINT("notice: read-lock: going for rw wait...\n");

	sem_wait(sem_rw);
	
	DEBUG_PRINT("notice: read-lock: done\n");
}

void read_unlock()
{
	DEBUG_PRINT("notice: read-unlock\n");

	sem_post(sem_rw);
}

void write_lock()
{
	int i;

	DEBUG_PRINT("notice: write-lock: going for mutex wait\n");

	// in case we have multiple writers
	sem_wait(sem_mutex);
	
	DEBUG_PRINT("notice: write-lock: going for write wait...\n");
			
	for(i = 0; i < MAX_READERS_AT_ONCE; i++)
	{
		DEBUG_PRINT("notice: write-lock: iteration %d\n", i);
		// we have a maximum number of readers
		// but we want to be able to read simultaneously
		sem_wait(sem_rw);
	}

	DEBUG_PRINT("notice: write-lock: going for mutex post...\n");

	// we shouldn't have problems with multiple writers now
	sem_post(sem_mutex);
	
	DEBUG_PRINT("notice: write-lock: done\n");
}

void write_unlock()
{
	DEBUG_PRINT("notice: write-unlock\n");
	
	int i;

	for(i = 0; i < MAX_READERS_AT_ONCE; i++)
	{
		// semaphore++
		sem_post(sem_rw);
	}
}
