#include <stdio.h>
#include "common.h"

// test.path.collection.test -> returns test.path.collection
int extract_collection(char collection[MAX_KEY_SIZE], char key[MAX_KEY_SIZE])
{
	int is_not_root = 1;
	int key_len = strlen(key);
	int i;

	// this shouldn't happen, but just in case...
	if(key_len > MAX_KEY_SIZE)
	{
		i = MAX_KEY_SIZE - 1;
	}
	else if(key_len == 0)
	{
		i = 0;
	}
	else
	{
		i = key_len - 1;
	}

	// run until we find our delimiter
	for(; i > 0 && key[i] != COLLECTION_DELIMITER; --i);
	
	// if we didn't find it, this is a key so it is not ok
	if(i == 0)
	{
		// root collection? where the string is something like "key" or ".key"
		is_not_root = 0;
	}
	else
	{
		// make a copy of our key and ensure it is well-formed
		strncpy(collection, key, i);
	}
	collection[i] = '\0';

	return is_not_root;
}

// test.path.collection.test -> returns collection
void extract_name(char name[MAX_KEY_SIZE], char path[MAX_KEY_SIZE])
{
	strcpy(name, path);

	// get position i of last delimiter
	// then do strncpy(dest, &path[i], strlen - i - 1)
	// dest[strlen - i] = 0
}

void print_error_case(int error)
{
	switch(error)
	{
		case ERR_USE:
			print_error(ERR_USE_MSG);
			break;
		case ERR_FORK:
			print_error(ERR_FORK_MSG);
			break;
		case ERR_CALLOC:
			print_error(ERR_CALLOC_MSG);
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
