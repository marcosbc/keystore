/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund and Adrian Marcelo Anillo
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

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
		DEBUG_PRINT("notice: store_init returned error_type %d\n", error_type);
	}
	else
	{
		error_type = ERR_USE;
		fprintf(stderr, USE_MSG, argv[0]);
	}

	return error_type;
}
