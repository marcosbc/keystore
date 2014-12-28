#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "common.h"

int socket_setup(char sockname[], int *s, struct sockaddr_un *addr)
{
	int len = 0;
	int error = 1;

	memset(addr, 0, sizeof(*addr));

	// create our socket
	if(-1 >= (*s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		perror("socket");
	}
	if(NULL != fopen(sockname, "r"))
	{
		fprintf(stderr, "error: socket connection already exists");
	}
	else
	{
		addr->sun_family = AF_UNIX;
		strcpy(addr->sun_path, sockname);

		len = sizeof(addr->sun_family) + (strlen(addr->sun_path) + 1);
		
		// bind...
		if (bind(*s, (struct sockaddr *) addr, len) < 0)
		{
			perror("bind");
		}
		else
		{
			error = 0;
		}
	}

	return error;
}

void free_data(void *p)
{
	p = NULL;
}

void print_error_case(int error)
{
	switch(error)
	{
		case ERR_FORK:
			print_error(ERR_FORK_MSG);
			break;
		case ERR_ALLOC:
			print_error(ERR_ALLOC_MSG);
			break;
		case ERR_STORE_SHMCREATE:
			print_error(ERR_STORE_SHMCREATE_MSG);
			break;
		case ERR_STORE_SHMLOAD:
			print_error(ERR_STORE_SHMLOAD_MSG);
			break;
		case ERR_STORE_SHMAT:
			print_error(ERR_STORE_SHMAT_MSG);
			break;
		case ERR_STORE_SHMDT:
			print_error(ERR_STORE_SHMDT_MSG);
			break;
		case ERR_STORE_SHMCTL:
			print_error(ERR_STORE_SHMCTL_MSG);
			break;
		case ERR_MEM_SEMOPEN:
			print_error(ERR_MEM_SEMOPEN_MSG);
			break;
		case ERR_MEM_SEMUNLINK:
			print_error(ERR_MEM_SEMUNLINK_MSG);
			break;
		case ERR_NONE:
		default:
			break;
	}
}

void print_error(char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
}

int min(int x, int y)
{
	return x < y ? x : y;
}

int max(int x, int y)
{
	return x > y ? x : y;
}
