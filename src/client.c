/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#include <stdio.h>
#include <sys/socket.h> // socket related things
#include <sys/un.h> // socket related things
#include <sys/stat.h> // mkfifo
#include <sys/shm.h> // 
#include <unistd.h> // fork
#include <fcntl.h> // O_CREAT, ...
#include <pthread.h>
#include "common.h"
#include "client.h"
#include "disk.h"
#include "database.h"
#include "sems.h"

int store_set(char key[], char *value, int num_dbs, char *dbs[])
{
	int error = ERR_NONE;
	char mode = '\0';
	int val_len = strlen(value);
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int len;
	int shmid = -1;
	store_info *store = NULL;
	key_t shm_key = ftok(".", KEY_ID);
	char *ack_msg = NULL; // stores the ack message
	char *ack_buff = NULL; // stores the response
	int max_db_len;
	int max_key_len;
	int ack_len;

	if(! sems_open())
	{
		error = ERR_MEM_SEMOPEN;
	}
	else
	{
		read_lock();
	}

	// get our server's public config information
	if(error == ERR_NONE &&
	   -1 == (shmid = shmget(shm_key, sizeof(struct info), 0664)))
	{
		error = ERR_STORE_SHMLOAD;
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
	}
	else if(NULL == (ack_msg = (char *) malloc(store->ack_len * sizeof(char)))
	     || NULL == (ack_buff = (char *) malloc(store->ack_len * sizeof(char))))
	{
		error = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("get first set of values from shm, store=%p\n", store);

		// set our mode and other variables
		mode = store->modes[STORE_MODE_SET_ID];
		DEBUG_PRINT("mode=%c\n", mode);
		max_db_len = store->max_db_len;
		DEBUG_PRINT("max_db_len=%d\n", max_db_len);
		max_key_len = store->max_key_len;
		DEBUG_PRINT("max_key_len=%d\n", max_key_len);
		ack_len = store->ack_len;
		DEBUG_PRINT("ack_len=%d\n", ack_len);
		strcpy(ack_msg, store->ack_msg);
		DEBUG_PRINT("ack_msg=%s\n", ack_msg);
		
		DEBUG_PRINT("get sock_path\n");

		// set sockaddr information values
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, store->sock_path);
		len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);

		DEBUG_PRINT("got server config pid=%d, acklen=%d ackmsg=%s socklen=%d \
sockpath=%s keylen=%d vallen=%d dblen=%d\n",
					 (int) store->pid, store->ack_len, store->ack_msg,
					 store->max_sock_len, store->sock_path, store->max_key_len,
					 store->max_val_len, store->max_db_len);

		// if we don't unlock before data is sent, the server and client
		// will block (this is only critical in this mode)
		read_unlock();
	}

	// initialize our socket
	if(error == ERR_NONE && -1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr, len) == -1)
	{
		print_perror("connect");
		error = ERR_CONNECT;
	}
	else if(NULL == (dbs_corrected = (char *) calloc(num_dbs * max_db_len,
	                                                 sizeof(char))) ||
	       (NULL == (key_corrected = (char *) calloc(max_key_len,
	                                                 sizeof(char)))))
	{
		error = ERR_ALLOC;
		print_perror("calloc");
	}
	else
	{
		// correct key and dbs variable's size
		strncpy(key_corrected, key, max_key_len - 1);
		key_corrected[max_key_len - 1] = '\0';

		for(i = 0; i < num_dbs; i++)
		{
			strncpy(dbs_corrected + i * max_db_len, dbs[i],
			        max_db_len- 1);
			*(dbs_corrected + (i + 1) * max_db_len - 1) = '\0';
		}

		// send the data
		DEBUG_PRINT("writing mode '%c' to server\n", mode);
		write(s, &mode, sizeof(char));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing key '%s' to server\n", key_corrected);
		write(s, key_corrected, max_key_len * sizeof(char));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing num_dbs '%d' to server\n", num_dbs);
		write(s, &num_dbs, sizeof(int));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing dbs[0] '%s' to server\n", dbs_corrected);
		write(s, dbs_corrected, num_dbs * max_db_len * sizeof(char));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing val_len '%d' to server\n", val_len);
		write(s, &val_len, sizeof(int));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing value '%s' to server\n", value);
		write(s, value, val_len * sizeof(char));
		
		DEBUG_PRINT("writing finished\n");
		
		// read the result (in this case it's another ack)
		// we can't read a recvd address since it would give segm error

		DEBUG_PRINT("---DONE---\n\n");
	}

	if(s >= 0)
	{
		// close our socket to stop communication
		close(s);
	}

	if(error != ERR_STORE_SHMLOAD && error != ERR_STORE_SHMAT
	   && -1 == shmdt(store))
	{
		error = ERR_STORE_SHMDT;
	}

	free(ack_buff);
	free(dbs_corrected);
	free(key_corrected);

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	int error = ERR_NONE;
	char mode = '\0';
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i = 0;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int len;
	int val_len;
	char *val = NULL;
	int shmid = -1;
	store_info *store = NULL;
	key_t shm_key = ftok(".", KEY_ID);
	char *ack_msg = NULL;
	char *ack_buff = NULL;
	int max_db_len;
	int max_key_len;
	int ack_len;

	if(! sems_open())
	{
		error = ERR_MEM_SEMOPEN;
	}
	else
	{
		read_lock();
	}

	// get our server's public config information
	if(error == ERR_NONE &&
	   -1 == (shmid = shmget(shm_key, sizeof(struct info), 0664)))
	{
		error = ERR_STORE_SHMLOAD;
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
	}
	else if(NULL == (ack_msg = (char *) malloc(store->ack_len * sizeof(char)))
	     || NULL == (ack_buff = (char *) malloc(store->ack_len * sizeof(char))))
	{
		error = ERR_ALLOC;
	}
	else
	{
		// set our mode and other variables
		mode = store->modes[STORE_MODE_GET_ID];
		max_db_len = store->max_db_len;
		max_key_len = store->max_key_len;
		ack_len = store->ack_len;
		strcpy(ack_msg, store->ack_msg);

		// set sockaddr information values
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, store->sock_path);
		len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);
		
		read_unlock();
	}

	// initialize our socket
	if(error == ERR_NONE && -1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
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
	else if(NULL == (dbs_corrected = (char *) calloc(num_dbs * max_db_len,
	                                                 sizeof(char))) ||
	       (NULL == (key_corrected = (char *) calloc(max_key_len,
	                                                 sizeof(char)))))
	{
		error = ERR_ALLOC;
		print_perror("calloc");
	}
	else
	{
		DEBUG_PRINT("copy key and dbs\n");

		// correct key and dbs variable's size
		strncpy(key_corrected, key, max_key_len - 1);
		key_corrected[max_key_len - 1] = '\0';

		// we will be using single pointers instead of tables to copy db
		for(i = 0; i < num_dbs; i++)
		{
			strncpy(dbs_corrected + i * max_db_len, dbs[i],
			        max_db_len - 1);
			*(dbs_corrected + (i + 1) * max_db_len - 1) = '\0';
			DEBUG_PRINT("got db \"%s\"\n",
			            dbs_corrected + i * max_db_len);
		}

		DEBUG_PRINT("writing to socket\n");

		// send the data
		DEBUG_PRINT("writing mode '%c' to server\n", mode);
		write(s, &mode, sizeof(char));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing key '%s' to server\n", key_corrected);
		write(s, key_corrected, max_key_len * sizeof(char));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing num_dbs '%d' to server\n", num_dbs);
		write(s, &num_dbs, sizeof(int));
		read(s, ack_buff, ack_len);
		DEBUG_PRINT("ack \"%s\" read\n", ack_buff);
		DEBUG_PRINT("writing dbs[0] '%s' to server\n", dbs_corrected);
		write(s, dbs_corrected, num_dbs * max_db_len * sizeof(char));

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
			write(s, ack_msg, ack_len);
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
				write(s, ack_msg, ack_len);
				DEBUG_PRINT("ack written\n");
	
				printf("%s: %s=%s\n", dbs_corrected + i * max_db_len,
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

	if(error != ERR_STORE_SHMLOAD && error != ERR_STORE_SHMAT
	   && -1 == shmdt(store))
	{
		error = ERR_STORE_SHMDT;
	}

	free(ack_buff);
	free(dbs_corrected);
	free(key_corrected);
	
	return error;
}

int store_halt()
{
	int error = ERR_NONE;
	char mode = '\0';
	int len;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int shmid = -1;
	key_t shm_key = ftok(".", KEY_ID);
	store_info *store = NULL;

	if(! sems_open())
	{
		error = ERR_MEM_SEMOPEN;
	}
	// get our server's public config information
	else if(error == ERR_NONE &&
	        -1 == (shmid = shmget(shm_key, sizeof(struct info), 0664)))
	{
		error = ERR_STORE_SHMLOAD;
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
	}
	else
	{
		read_lock();
		
		// set our mode
		mode = store->modes[STORE_MODE_STOP_ID];

		// set sockaddr information values
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, store->sock_path);
		len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);
		
		read_unlock();
	}
	
	DEBUG_PRINT("notice: database preparing to shut down\n");

	// initialize our socket
	if(error == ERR_NONE && -1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
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
