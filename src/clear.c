#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include "common.h"

int main()
{
	int error = 0;
	key_t shm_key = ftok(".", KEY_ID);
	int shmid = shmget(shm_key, sizeof(struct info), 0664);

	if(-1 == shmid)
	{
		print_perror("shmget");
		error = 1;
	}
	else if(-1 == shmctl(shmid, IPC_RMID, NULL))
	{
		print_perror("shmctl");
		error = 1;
	}

	return error;
}

