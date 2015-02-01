#include <stdio.h>
#include <stdarg.h> // va_list and related functions
#include "common.h"

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
