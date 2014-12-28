/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#include <stdio.h>
#include <sys/shm.h>
#include <sys/socket.h> // socket related things
#include <sys/un.h> // socket related things
#include <pthread.h>
#include <sys/stat.h> // mkfifo
#include <unistd.h> // fork
#include <fcntl.h> // O_CREAT, ...
#include <sys/un.h>
#include "common.h"
#include "client.h"
#include "memory.h"
#include "disk.h"
#include "database.h"

int store_set(char key[], char value[], int num_dbs, char *dbs[])
{
	int error = 0;
	char mode = STORE_MODE_SET;
	int val_len = strlen(value);
	int shmid = 0;
	key_t shm_key = ftok(".", KEY_ID);
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int len;
	store_entry *result = NULL;
	store_info *store = NULL;

	// set sockaddr information values
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, STORE_SOCKET_PATH);
	len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);
	memset(&addr, 0, sizeof(addr));

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif

	// get our memory location (our first root database with root values)
	// note we only need to do this if we are going to get-set via memory
	if(-1 == (shmid = shmget(shm_key, sizeof(struct info), 0666)))
	{
		error = ERR_STORE_SHMLOAD;
		perror("shmget");
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
		perror("shmat");
	}
	// initialize our socket
	else if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr, len) == -1)
	{
		perror("connect");
		error = ERR_CONNECT;
	}
	else if(NULL == (dbs_corrected = (char *) malloc(num_dbs * MAX_DB_SIZE
	                                                 * sizeof(char))) ||
	       (NULL == (key_corrected = (char *) malloc(strlen(key)
	                                                    * sizeof(char)))))
	{
		error = ERR_ALLOC;
		perror("malloc");
	}
	else
	{
		#ifdef __DEBUG__
		print_existing_databases(store->dbs);
		#endif

		// correct key and dbs variable's size
		strncpy(key_corrected, key, MAX_KEY_SIZE - 1);
		key_corrected[MAX_VAL_SIZE - 1] = '\0';

		for(i = 0; i < num_dbs; i++)
		{
			strncpy(dbs_corrected + i * MAX_DB_SIZE, dbs[i], MAX_DB_SIZE - 1);
			*(dbs_corrected + (i + 1) * MAX_DB_SIZE - 1) = '\0';
		}

		// send the data
		write(s, &mode, sizeof(char));
		write(s, key, MAX_KEY_SIZE * sizeof(char));
		write(s, &num_dbs, sizeof(int));
		write(s, *dbs, num_dbs * MAX_DB_SIZE * sizeof(char));
		write(s, &val_len, sizeof(int));
		write(s, value, val_len * sizeof(char));
		
		DEBUG_PRINT("writing finished\n");
		
		// read the address
		read(s, &result, sizeof(result));

		DEBUG_PRINT("got address %p from server\n", result);

		// read entry location (which is persistent)
	}

	if(s >= 0)
	{
		// close our socket to stop communication
		close(s);
	}

	free(dbs_corrected);
	free(key_corrected);

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	int error = 0;
	char mode = STORE_MODE_GET;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int len;
	store_entry *result = NULL;
	store_info *store = NULL;

	// set sockaddr information values
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, STORE_SOCKET_PATH);
	len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);
	memset(&addr, 0, sizeof(addr));

	#ifdef __DEBUG__
	print_databases(num_dbs, dbs);
	#endif

	// get our memory location (our first root database with root values)
	// note we only need to do this if we are going to get-set via memory
	if((shmid = shmget(shm_key, sizeof(struct info), 0666)) == -1)
	{
		error = ERR_STORE_SHMLOAD;
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
	}
	// initialize our socket
	else if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr, len) == -1)
	{
		perror("connect");
		error = ERR_CONNECT;
	}
	else if(NULL == (dbs_corrected = (char *) malloc(num_dbs * MAX_DB_SIZE
	                                                 * sizeof(char))) ||
	       (NULL == (key_corrected = (char *) malloc(strlen(key)
	                                                    * sizeof(char)))))
	{
		error = ERR_ALLOC;
		perror("malloc");
	}
	else
	{
		#ifdef __DEBUG__
		print_existing_databases(store->dbs);
		#endif

		// correct key and dbs variable's size
		strncpy(key_corrected, key, MAX_KEY_SIZE - 1);
		key_corrected[MAX_VAL_SIZE - 1] = '\0';

		// we will be using single pointers instead of tables to copy db
		for(i = 0; i < num_dbs; i++)
		{
			strncpy(dbs_corrected + i * MAX_DB_SIZE, dbs[i], MAX_DB_SIZE - 1);
			*(dbs_corrected + (i + 1) * MAX_DB_SIZE - 1) = '\0';
			DEBUG_PRINT("got db \"%s\"\n", dbs_corrected + i * MAX_DB_SIZE);
		}

		DEBUG_PRINT("writing to socket\n");

		// send the data
		write(s, &mode, sizeof(char));
		write(s, key, MAX_KEY_SIZE * sizeof(char));
		write(s, &num_dbs, sizeof(int));
		write(s, dbs_corrected, num_dbs * MAX_DB_SIZE * sizeof(char));

		DEBUG_PRINT("writing finished\n");
		
		// read the address
		read(s, &result, sizeof(result));

		DEBUG_PRINT("got address %p from server\n", result);
	}

	if(s >= 0)
	{
		// close and unlink our socket
		close(s);
	}

	free(dbs_corrected);
	free(key_corrected);
	
	return error;
}

int store_halt()
{
	int error = 0;
	char mode = STORE_MODE_STOP;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);
	int len;
	int s = -1; // client socket
	struct sockaddr_un addr;
	store_info *store = NULL;

	// set sockaddr information values
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, STORE_SOCKET_PATH);
	len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);
	memset(&addr, 0, sizeof(addr));

	DEBUG_PRINT("notice: database preparing to shut down\n");

	// get our memory location (our first root database with root values)
	// note we only need to do this if we are going to get-set via memory
	if((shmid = shmget(shm_key, sizeof(struct info), 0666)) == -1)
	{
		error = ERR_STORE_SHMLOAD;
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
	}
	// initialize our socket
	else if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr, len) == -1)
	{
		perror("connect");
		error = ERR_CONNECT;
	}
	else
	{
		// send halt character to daemon (for shutdown)
		write(s, &mode, sizeof(char));

		DEBUG_PRINT("notice: database shutting down\n");
	}

	if(s >= 0)
	{
		// close and unlink our socket
		close(s);
	}

	return error;
}
