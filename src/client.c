/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#include <stdio.h>
#include <sys/shm.h>
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
	char mode = STORE_PIPE_MODE_SET;
	int val_len = strlen(value);
	int shmid = 0;
	key_t shm_key = ftok(".", KEY_ID);
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i;
	char *ok_msg = NULL;

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
	else if(store->pid <= (int) 0)
	{
		error = ERR_PID_NUM;
	}
	// initialize our socket
	else if()
	{

	}
	else if(NULL == (dbs_corrected = (char *) malloc(num_dbs * MAX_DB_SIZE
	                                                      * sizeof(char))))
	{
		error = ERR_ALLOC;
	}
	// alloc once at a time...
	else if(NULL == (key_corrected = (char *) malloc(strlen(key)
	                                                     * sizeof(char))))
	{
		error = ERR_ALLOC;
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

		// send the signal to our process, and then send the data
		kill(store->pid, SIGUSR1);
		write(fifo_pipe, &mode, sizeof(char));
		write(fifo_pipe, key, MAX_KEY_SIZE * sizeof(char));
		write(fifo_pipe, &num_dbs, sizeof(int));
		write(fifo_pipe, *dbs, num_dbs * MAX_DB_SIZE * sizeof(char));
		write(fifo_pipe, &val_len, sizeof(int));
		write(fifo_pipe, value, val_len * sizeof(char));
		
		DEBUG_PRINT("writing to pipe finished, reading ok/ack msg\n");
		
		// read the ok/ack message
		read(fifo_pipe_ack, STORE_PIPE_ACK_MSG, STORE_PIPE_ACK_MSG_LEN);

		// if ok == MSG

		DEBUG_PRINT("ok msg received: \"%s\"\n", ok_msg);

		// read entry location (which is persistent)
	}

	if(-1 != fifo_pipe)
	{
		// close and unlink our pipe
		close(fifo_pipe);
	}

	if(-1 != fifo_pipe_ack)
	{
		// close and unlink our pipe
		close(fifo_pipe_ack);
	}

	unlink(STORE_PIPE_ACK);

	free(dbs_corrected);
	free(key_corrected);

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	char mode = STORE_PIPE_MODE_GET;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);
	char *dbs_corrected = NULL;
	char *key_corrected = NULL;
	int i;
	char *ok_msg = NULL;
	
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
	// open our pipe
	else if(store->pid <= (int) 0)
	{
		error = ERR_PID_NUM;
	}
	else if(NULL == (dbs_corrected = (char *) malloc(num_dbs * MAX_DB_SIZE
	                                                      * sizeof(char))))
	{
		error = ERR_ALLOC;
	}
	// alloc once at a time...
	else if(NULL == (key_corrected = (char *) malloc(strlen(key)
	                                                     * sizeof(char))))
	{
		error = ERR_ALLOC;
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

		DEBUG_PRINT("writing to pipe\n");

		// send the signal to our process, and then send the data
		kill(store->pid, SIGUSR2);
		write(fifo_pipe, &mode, sizeof(char));
		write(fifo_pipe, key, MAX_KEY_SIZE * sizeof(char));
		write(fifo_pipe, &num_dbs, sizeof(int));
		write(fifo_pipe, dbs_corrected, num_dbs * MAX_DB_SIZE * sizeof(char));

		DEBUG_PRINT("writing to pipe finished, reading ok/ack msg\n");
		
		// read the ok/ack message
		read(fifo_pipe_ack, ok_msg, STORE_PIPE_ACK_MSG_LEN);
	
		// if ok == MSG

		DEBUG_PRINT("ok msg received: \"%s\"\n", ok_msg);

		// read entry location (which is persistent)

		DEBUG_PRINT("pipe closed\n");
	}

	if(-1 != fifo_pipe)
	{
		// close and unlink our pipe
		close(fifo_pipe);
	}

	if(-1 != fifo_pipe_ack)
	{
		// close and unlink our pipe
		close(fifo_pipe_ack);
	}

	unlink(STORE_PIPE_ACK);

	free(dbs_corrected);
	free(key_corrected);
	
	return error;
}

int store_halt()
{
	char mode = STORE_PIPE_MODE_STOP;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);

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
	// open our pipe
	else if(-1 == (fifo_pipe = open(STORE_PIPE, O_WRONLY | O_NONBLOCK)))
	{
		error = ERR_FIFO_OPEN;
	}
	else if(store->pid <= (int) 0)
	{
		error = ERR_PID_NUM;
	}
	else
	{
		DEBUG_PRINT("kill sent to pid %d\n", (int) store->pid);
		kill(store->pid, SIGINT);
		write(fifo_pipe, &mode, sizeof(char));

		DEBUG_PRINT("notice: database shutting down\n");
	}

	return error;
}

#ifdef __DEBUG__
void print_databases(int num_dbs, char *dbs[])
{
	int i;

    printf("\nlist of databases:\n------------------\n");
    for(i = 0; i < num_dbs; i++)
    {
        printf("database#%d: %s\n", i, dbs[i]);
    }
    printf("-----------------\n\n");
}

void print_existing_databases(store_db *store_dbs)
{
	int i = 0;

    printf("\nlist of databases:\n------------------\n");
    while(store_dbs != NULL)
    {
		if(store_dbs->name != NULL)
		{
			printf("database#%d: %s\n", i, store_dbs->name);
		}
		else
		{
			printf("database#%d (unnamed)\n", i);
		}
		i++;
		store_dbs = store_dbs->next;
    }
    printf("------------------\n\n");

}
#endif
