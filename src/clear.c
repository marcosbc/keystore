/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund and Adrian Marcelo Anillo
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#include <stdio.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"
#include "sems.h"

int main()
{
	int error = 0;
	// sem_t *sem_rw = (sem_t *) -1;
	// sem_t *sem_mutex = (sem_t *) -1;
	key_t shm_key = ftok(".", KEY_ID);
	int shmid = shmget(shm_key, sizeof(struct info), 0664);

	if(-1 != shmid && -1 == shmctl(shmid, IPC_RMID, NULL))
	{
		print_perror("shmctl");
		error = 1;
	}

	unlink(SEM_RW);
	unlink(SEM_MUTEX);
	
	// unlink our socket
	unlink(STORE_SOCKET_PATH);

	return error;
}
