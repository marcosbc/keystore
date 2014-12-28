/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "client.h"
#include "keystore.h"

// modes:
// - create
// - set
// - get

// fork:
// - parent: alter the database in memory
// - children: alter the database file 

// threads:
//   for searching through multiple databases

// semaphores:
//   for altering memory operations

int main(int argc, char *argv[])
{
	int error_type = 0;
	int mode = 0;
	int arguments_ok = check_arguments(argc, argv, &mode);

	DEBUG_PRINT("notice: check_arguments() returned %d\n", arguments_ok);

	// make sure our arguments get passed correctly
	if(arguments_ok)
	{
		DEBUG_PRINT("notice: arguments are ok, with mode %d\n", mode);
		if(mode == MODE_SET_ID)
		{
			// we are setting a value in a database
			error_type = store_set(argv[SET_KEY],
			                       argv[SET_VAL],
			                       argc - SET_NAMES,  // number of dbs
					               &argv[SET_NAMES]); // db names in array
			DEBUG_PRINT("notice: store_set returned error_type %d\n", error_type);
		}
		else if(mode == MODE_GET_ID)
		{
			// we are getting a value in a database
			error_type = store_get(argv[GET_KEY],
			                       argc - GET_NAMES,
			                       &argv[GET_NAMES]);
			DEBUG_PRINT("notice: store_get returned error_type %d\n", error_type);
		}
		else
		{
			error_type = store_halt();
			DEBUG_PRINT("notice: store_halt returned error_type %d\n", error_type);
		}
	}
	else
	{
		error_type = ERR_USE;
		printf(ERR_USE_MSG, argv[0]);
	}

	print_error_case(error_type);

	return error_type;
}

int check_arguments(int num, char *args[], int *mode)
{
	int ok = 1;

	// check if we entered any argument
	if(num > 1)
	{
		// is the mode and number of arguments ok?
		if(0 == strcmp(args[MODE], MODE_SET) && num >= SET_MIN_NUM_ARGS)
		{
			*mode = MODE_SET_ID;
		}
		else if(0 == strcmp(args[MODE], MODE_GET) && num >= GET_MIN_NUM_ARGS)
		{
			*mode = MODE_GET_ID;
		}
		else if(0 == strcmp(args[MODE], MODE_HALT) && num == HALT_NUM_ARGS)
		{
			*mode = MODE_HALT_ID;
		}
		else
		{
			ok = 0;
		}
	}
	else
	{
		ok = 0;
	}

	return ok;
}
