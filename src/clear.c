#include <sys/shm.h>
#include <semaphore.h>
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
