#include <stdio.h>
#include <string.h>
#include "common.h"
#include "server.h"
#include "daemon.h"

int main(int argc, char *argv[])
{
	int error_type = 0;

	// make sure our arguments get passed correctly
	if(argc == NUM_ARGS)
	{
		error_type = store_server_init();
		DEBUG_PRINT("notice: store_init returned error %d\n", error_type);
	}
	else
	{
		error_type = ERR_USE;
		fprintf(stderr, USE_MSG, argv[0]);
	}

	return error_type;
}
