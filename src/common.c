#include <stdarg.h> // va_list and related functions
#include <sys/time.h>
#include "common.h"

void print_common_error_case(int error)
{
	switch(error)
	{
		// common errors between server and client
		case ERR_ALLOC:
			print_error(ERR_ALLOC_MSG);
			break;
		case ERR_SOCKET:
			print_error(ERR_SOCKET_MSG);
			break;
		case ERR_SHMAT:
			print_error(ERR_SHMAT_MSG);
			break;
		case ERR_SHMDT:
			print_error(ERR_SHMDT_MSG);
			break;
		case ERR_SEMOPEN:
			print_error(ERR_SEMOPEN_MSG);
			break;
		case ERR_SEMCLOSE:
			print_error(ERR_SEMCLOSE_MSG);
			break;
		case ERR_INVALID_MODE:
			print_error(ERR_INVALID_MODE_MSG);
			break;

		case ERR_NONE:
		default:
			break;
	}
}

// return the time difference in milliseconds
double time_diff(struct timeval start, struct timeval end)
{
	double start_time = (double) start.tv_sec * 1000000 + (double) start.tv_usec;
	double end_time = (double) end.tv_sec * 1000000 + (double) end.tv_usec;

	return (end_time - start_time) / 1000;
}

void print_error(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	vfprintf(stderr, "\033[31merror: ", args);
	vfprintf(stderr, msg, args);
	vfprintf(stderr, "\n\033[0m", args);

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
