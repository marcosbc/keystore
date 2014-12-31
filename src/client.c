/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#include <stdio.h>
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
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int len;
	char ack_buff[STORE_ACK_LEN];
	char sock_path[MAX_SOCK_PATH_SIZE];

	// set up our socket path
	getcwd(sock_path, sizeof(sock_path));
	strcat(sock_path, "/");
	strcat(sock_path, STORE_SOCKET_PATH);

	// set sockaddr information values
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sock_path);
	len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);

	// initialize our socket
	if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr, len) == -1)
	{
		print_perror("connect");
		error = ERR_CONNECT;
	}
	else if(NULL == (dbs_corrected = (char *) calloc(num_dbs * MAX_DB_SIZE,
	                                                 sizeof(char))) ||
	       (NULL == (key_corrected = (char *) calloc(strlen(key),
	                                                 sizeof(char)))))
	{
		error = ERR_ALLOC;
		print_perror("calloc");
	}
	else
	{
		// correct key and dbs variable's size
		strncpy(key_corrected, key, MAX_KEY_SIZE - 1);
		key_corrected[MAX_KEY_SIZE - 1] = '\0';

		for(i = 0; i < num_dbs; i++)
		{
			strncpy(dbs_corrected + i * MAX_DB_SIZE, dbs[i], MAX_DB_SIZE - 1);
			*(dbs_corrected + (i + 1) * MAX_DB_SIZE - 1) = '\0';
		}

		// send the data
		DEBUG_PRINT("writing mode '%c' to server\n", mode);
		write(s, &mode, sizeof(char));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing key '%s' to server\n", key_corrected);
		write(s, key_corrected, MAX_KEY_SIZE * sizeof(char));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing num_dbs '%d' to server\n", num_dbs);
		write(s, &num_dbs, sizeof(int));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing dbs[0] '%s' to server\n", dbs_corrected);
		write(s, dbs_corrected, num_dbs * MAX_DB_SIZE * sizeof(char));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing val_len '%d' to server\n", val_len);
		write(s, &val_len, sizeof(int));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing value '%s' to server\n", value);
		write(s, value, val_len * sizeof(char));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		
		DEBUG_PRINT("writing finished\n");
		
		// read the result (in this case it's another ack)
		// we can't read a recvd address since it would give segm error
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);

		DEBUG_PRINT("---DONE---\n\n");
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
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i = 0;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int len;
	char ack_buff[STORE_ACK_LEN];
	int val_len;
	char *val = NULL;
	char sock_path[MAX_SOCK_PATH_SIZE];

	// set up our socket path
	getcwd(sock_path, sizeof(sock_path));
	strcat(sock_path, "/");
	strcat(sock_path, STORE_SOCKET_PATH);

	// set sockaddr information values
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sock_path);
	len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);

	// initialize our socket
	if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		DEBUG_PRINT("error at socket creation\n");
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr, len) == -1)
	{
		print_perror("connect");
		error = ERR_CONNECT;
	}
	else if(NULL == (dbs_corrected = (char *) calloc(num_dbs * MAX_DB_SIZE,
	                                                 sizeof(char))) ||
	       (NULL == (key_corrected = (char *) calloc(strlen(key),
	                                                 sizeof(char)))))
	{
		error = ERR_ALLOC;
		print_perror("calloc");
	}
	else
	{
		DEBUG_PRINT("copy key and dbs\n");

		// correct key and dbs variable's size
		strncpy(key_corrected, key, MAX_KEY_SIZE - 1);
		key_corrected[MAX_KEY_SIZE - 1] = '\0';

		// we will be using single pointers instead of tables to copy db
		for(i = 0; i < num_dbs; i++)
		{
			strncpy(dbs_corrected + i * MAX_DB_SIZE, dbs[i], MAX_DB_SIZE - 1);
			*(dbs_corrected + (i + 1) * MAX_DB_SIZE - 1) = '\0';
			DEBUG_PRINT("got db \"%s\"\n", dbs_corrected + i * MAX_DB_SIZE);
		}

		DEBUG_PRINT("writing to socket\n");

		// send the data
		DEBUG_PRINT("writing mode '%c' to server\n", mode);
		write(s, &mode, sizeof(char));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing key '%s' to server\n", key_corrected);
		write(s, key_corrected, MAX_KEY_SIZE * sizeof(char));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing num_dbs '%d' to server\n", num_dbs);
		write(s, &num_dbs, sizeof(int));
		read(s, ack_buff, STORE_ACK_LEN);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing dbs[0] '%s' to server\n", dbs_corrected);
		write(s, dbs_corrected, num_dbs * MAX_DB_SIZE * sizeof(char));

		DEBUG_PRINT("writing finished\n");
		
		for(i = 0; i < num_dbs && error == ERR_NONE; i++)
		{
			DEBUG_PRINT("\nREAD ITERATION %d\n", i);

			// read the value result
			// we can't read the address because it's not shared and it would
			// give a segfault error
			// for(i = 0; i < num_dbs; i++) entries[i];...
			read(s, &val_len, sizeof(int));
			DEBUG_PRINT("val_len \"%d\" read\n", val_len);
			write(s, STORE_ACK, STORE_ACK_LEN);
			DEBUG_PRINT("ack written\n");
			
			DEBUG_PRINT("current value pointer before calloc for %d bytes: %p\n",
			            (val_len + 1) * ((int) sizeof(char)), val);
			val = (char *) calloc((val_len + 1), sizeof(char));
			DEBUG_PRINT("finished calloc\n");
			DEBUG_PRINT("value pointer after calloc: %p\n", val);

			// memory alloc
			if(NULL != val)
			{
				DEBUG_PRINT("reading value\n");
				read(s, val, (val_len + 1) * sizeof(char));
				DEBUG_PRINT("val \"%s\" read\n", val);
				write(s, STORE_ACK, STORE_ACK_LEN);
				DEBUG_PRINT("ack written\n");
	
				printf("%s: %s=%s\n", dbs_corrected + i * MAX_DB_SIZE,
				       key_corrected, val);
				DEBUG_PRINT("---DONE---\n");
			}
			else
			{
				print_perror("calloc");
				error = ERR_ALLOC;
			}
			free(val);
			val = NULL;
			val_len = 0;
		}
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
	int len;
	int s = -1; // client socket
	struct sockaddr_un addr;
	char sock_path[MAX_SOCK_PATH_SIZE];

	// set up our socket path
	getcwd(sock_path, sizeof(sock_path));
	strcat(sock_path, "/");
	strcat(sock_path, STORE_SOCKET_PATH);

	// set sockaddr information values
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sock_path);
	len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);

	DEBUG_PRINT("notice: database preparing to shut down\n");

	// initialize our socket
	if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr, len) == -1)
	{
		print_perror("connect");
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
