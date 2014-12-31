#include <stdio.h>
#include <stdarg.h> // va_list and related functions
#include "common.h"

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

void print_error(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	vfprintf(stderr, "\033[31merror:", args);
	vfprintf(stderr, msg, args);
	vfprintf(stderr, "\033[0m", args);

	va_end(args);
}

void print_perror(const char *msg)
{
	fprintf(stderr, "\033[31m");
	perror(msg);
	fprintf(stderr, "\033[0m");
}

int min(int x, int y)
{
	return x < y ? x : y;
}

int max(int x, int y)
{
	return x > y ? x : y;
}
