#include <semaphore.h>
#include <fcntl.h> // O_CREAT, ...
#include "common.h"
#include "sems.h"

sem_t *sem_mutex = NULL;
sem_t *sem_rw = NULL;
int num_rd = 0;

int sems_init()
{
	int ok = 1;
	
	DEBUG_PRINT("notice: creating semaphores for memory operations\n");

	// if we don't unlink them before, they will give problems if they exist
	sem_unlink(SEM_RW);
	sem_unlink(SEM_MUTEX);

	// create semaphores
	sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1); // binary semaphore
	sem_rw = sem_open(SEM_RW, O_CREAT, 0666, 1);

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
	DEBUG_PRINT("notice: read-lock: going for mutex wait...\n");

	sem_wait(sem_mutex);

	// first reader? block writing
	if(++num_rd == 1)
	{
		DEBUG_PRINT("notice: read-lock: first reader, write wait...\n");

		sem_wait(sem_rw);
	}

	sem_post(sem_mutex);

	DEBUG_PRINT("notice: read-lock: done\n");
}

void read_unlock()
{
	DEBUG_PRINT("notice: read-unlock: going for mutex wait...\n");

	sem_wait(sem_mutex);

	// last reader?
	if(--num_rd == 0)
	{
		DEBUG_PRINT("notice: read-unlock: last reader, write post...\n");

		// allow writing
		sem_post(sem_rw);
	}

	sem_post(sem_mutex);

	DEBUG_PRINT("notice: read-unlock: done");
}

void write_lock()
{
	DEBUG_PRINT("notice: write-lock\n");

	sem_wait(sem_rw);
}

void write_unlock()
{
	DEBUG_PRINT("notice: write-unlock\n");

	sem_post(sem_rw);
}
