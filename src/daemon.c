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
		fprintf(stderr, ERR_USE_MSG, argv[0]);
	}
	print_error_case(error_type);

	return error_type;
}

void print_error_case(int error)
{
	switch(error)
	{
		case ERR_SESSION:
			print_error(ERR_SESSION_MSG);
			break;
		case ERR_SHMCREATE:
			print_error(ERR_SHMCREATE_MSG);
			break;
		case ERR_SHMCTL:
			print_error(ERR_SHMCTL_MSG);
			break;
		case ERR_SEMUNLINK:
			print_error(ERR_SEMUNLINK_MSG);
			break;
		case ERR_THR:
			print_error(ERR_THR_MSG);
			break;
		case ERR_THRJOIN:
			print_error(ERR_THRJOIN_MSG);
			break;
		case ERR_BIND:
			print_error(ERR_BIND_MSG);
			break;
		case ERR_LISTEN:
			print_error(ERR_LISTEN_MSG);
			break;
		case ERR_ACCEPT:
			print_error(ERR_ACCEPT_MSG);
			break;

		default:
			print_common_error_case(error);
		case ERR_NONE:
			break;
	}
}
