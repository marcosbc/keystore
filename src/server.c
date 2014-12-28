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
#include <signal.h>
#include "common.h"
#include "server.h"
#include "memory.h"
#include "disk.h"
#include "database.h"

int stop_server = 0; // for signals to work

void store_stop()
{
	stop_server = 1;
}

store_entry *store_write(char key[MAX_KEY_SIZE], char *val, int num_dbs,
                         char *dbs, store_info **store)
{
	/** ERRORS **/

	int err = 0;
	int i = 0;
	pid_t pid;
	int therr = 0;
	pthread_t *thids = NULL;
	struct entry_inf *ent_inf = NULL;
	store_entry *entry = NULL;
	// FILE **fids = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", value \"%s\", num_dbs %d\n",
	            key, val, num_dbs);

	// the parent process modifies the memory so we can print our results
	// faster and let the other process continue writing to disk
	if(1) // if((pid = fork()) > 0) // add mode=parent|children in args
	{
		thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t));
		ent_inf = (struct entry_inf *) calloc(num_dbs,
		                                      sizeof(struct entry_inf));
		if(thids == NULL || ent_inf == NULL)
		{
			err = ERR_ALLOC;
		}
		else
		{
			DEBUG_PRINT("alloc ok \n");

			// parent - alter the database in memory
			for(; i < num_dbs && ! therr && err == ERR_NONE; i++)
			{
				// create the entry information for setting
				ent_inf[i].key = key;
				ent_inf[i].value = val;
				ent_inf[i].db_name = dbs + i * MAX_DB_SIZE;
				ent_inf[i].entry = NULL;
				ent_inf[i].dbs = (*store)->dbs;
				ent_inf[i].error = 0;
				
				// create our thread
				therr = pthread_create(&thids[i], NULL, memory_set,
				                       &ent_inf[i]);

				DEBUG_PRINT("notice: [parent] thread#%d %d (\"%s\" => \
\"%s\") to insert in db \"%s\"\n",
				            i, (int) thids[i], key, "",//val,
							ent_inf[i].db_name);

				if(therr != ERR_NONE) 
				{
					err = ERR_THR;
				}
			}
		}

		// set the number of iterations that went correctly
		num_dbs = i;
		therr = 0;
		
		// now, end our threads
		for(i = 0; i < num_dbs && therr == ERR_NONE && err == ERR_NONE;
		    i++)
		{
			DEBUG_PRINT("notice: [parent] ending thread#parent-%d %d...\n",
			            i, (int) thids[i]);
				
			therr = pthread_join(thids[i], (void **) &err);

			if(therr != 0)
			{
				err = ERR_THRJOIN;
			}
			else
			{
				entry = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: [parent] ended thread#parent-%d %d\
returned value %d\n",
			                i, (int) thids[i], ent_inf[i].error);
			}
		}

		// unmap our shared memory
		if(err != ERR_STORE_SHMLOAD && -1 == shmdt(*store))
		{
			err = ERR_STORE_SHMDT;
		}

		free(ent_inf);
		free(thids);
	}
	else if(pid == 0)
	{
		// *** PARENT *** 
	}
	else
	{
		err = ERR_FORK;
	}

	return entry;
}

store_entry *store_read(char key[MAX_KEY_SIZE], int num_dbs, char *dbs,
                        store_info *store)
{
	int err = 0;
	int i;
	int therr = 0;
	pthread_t *thids = NULL;
	struct entry_inf *ent_inf = NULL;
	store_entry *entry = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", num_dbs %d\n",
	            key, num_dbs);
	
	// create our thread entries
	thids = (pthread_t *) calloc(num_dbs,
	                             sizeof(pthread_t));
	ent_inf = (struct entry_inf *) calloc(num_dbs,
	                                      sizeof(struct entry_inf));
	
	if(thids == NULL || ent_inf == NULL)
	{
		err = ERR_ALLOC;
	}
	else
	{
		for(i = 0; i < num_dbs && ! therr && err == ERR_NONE; i++)
		{
			// create the entry information for setting
			ent_inf[i].key = key;
			ent_inf[i].value = NULL;
			ent_inf[i].db_name = dbs + i * MAX_DB_SIZE;
			ent_inf[i].entry = NULL;
			ent_inf[i].dbs = store->dbs;
			ent_inf[i].error = 0;

			// create our thread -> ERROR???
			therr = pthread_create(&thids[i], NULL, memory_get,
			                       &ent_inf[i]);

			// check val, it's null probably
			DEBUG_PRINT("notice: thread#%d %d (key \"%s\" \
 to search in db \"%s\"\n",
			            i, (int) thids[i], key, ent_inf[i].db_name);

			if(therr != 0)
			{
				err = ERR_THR;
			}
		}
	
		// set our number of iterations that went correctly
		num_dbs = i;

		// end our threads
		for(i = 0; i < num_dbs && err == ERR_NONE; i++)
		{
			DEBUG_PRINT("notice: ending thread#%d %d...\n",
			            i, (int) thids[i]);
	
			therr = pthread_join(thids[i], NULL);

			if(therr != 0)
			{
				DEBUG_PRINT("thread ended, error %d\n", therr);
				err = ERR_THRJOIN;
			}
			else
			{
				entry = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: ended thread#%d %d, \
returned %d\n",
				            i, (int) thids[i], err);
			}
		}

		// unmap our shared memory
		if(-1 == shmdt(store))
		{
			err = ERR_STORE_SHMDT;
		}
	}

	free(thids);
	free(ent_inf);

	return entry;
}

int store_server_act(int s, store_info **store)
{
	int error = 0;
	int remote_s = -1; // remote socket
	struct sockaddr_un remote;
	socklen_t remote_len = sizeof(remote);
	char mode;
	char key[MAX_VAL_SIZE];
	uint32_t num_dbs; // 32-bit and 64-bit intercompatibility
	char *dbs;
	uint32_t val_len = 0; // 32-bit and 64-bit intercompatibility
	char *val = NULL;
	store_entry *result = (store_entry *) -1;

	DEBUG_PRINT("waiting for a connection...\n");

	// accept connection (if it fails, it is because server was closed: no error)
	if(-1 != (remote_s = accept(s, (struct sockaddr *) &remote, &remote_len)))
	{
		// read data
		read(remote_s, &mode, sizeof(char));

		if(mode == STORE_MODE_SET || mode == STORE_MODE_GET)
		{
			read(remote_s, &key, MAX_VAL_SIZE * sizeof(char));
			read(remote_s, &num_dbs, sizeof(uint32_t));
			
			dbs = (char *) malloc(num_dbs * MAX_DB_SIZE * sizeof(char));
			if(dbs == NULL)
			{
				perror("malloc");
				error = ERR_ALLOC;
			}
			else
			{
				read(remote_s, dbs, num_dbs * MAX_DB_SIZE * sizeof(char));

				// if we're setting a value, we must also read it
				if(mode == STORE_MODE_SET)
				{
					// we don't know how big our value will be
					// val_len + 1 -> we must allow the \0 to be the last char
					val = (char *) malloc((val_len + 1) * sizeof(char));
			
					if(val == NULL)
					{
						perror("malloc");
						error = ERR_ALLOC;
					}
					else
					{
						/** PLEASE LOOK */
						/* val_len = */read(remote_s, val, val_len * sizeof(char));
						val[val_len] = '\0'; // ensure we reach end of string

						// now that we have everything, call function for setting
						result = store_write(key, val, num_dbs, dbs, store);
					}
				}
				// 'get' mode
				else
				{
					// now that we have everything, call the function getting
					result = store_read(key, num_dbs, dbs, *store);
				}
			}
		}
		// formal server stop so no error
		else
		{
			store_stop();
		}

		// send the result pointer address to our receiver
		write(remote_s, &store, sizeof(store_entry *));

		// close connection
		close(remote_s);

		DEBUG_PRINT("connection closed\n");
	}

	return error;
}

int store_server_init()
{
	int error = 0;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);
	struct sigaction act;
	int s = -1; // our socket
	struct sockaddr_un addr;
	store_info *store = NULL;

	// set up signal to stop server
	act.sa_handler = store_stop;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);

	DEBUG_PRINT("notice: database starting\n");

	// ignore our sigpipe signal
	signal(SIGPIPE, SIG_IGN);
	
	// create our root db
	shmid = shmget(shm_key, sizeof(struct info), IPC_CREAT | IPC_EXCL | 0666);

	// we shouldn't be able to initialize memory when it has already been done
	if(-1 == shmid)
	{
		error = ERR_STORE_SHMCREATE;
	}
	if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
	}
	else if(! memory_init())
	{
		error = ERR_MEM_SEMOPEN;
		perror("sem_open");
	}
	// create our socket
	else if(socket_setup(SERVER_SOCKET_ADDRESS, &s, &addr))
	{
		error = ERR_CONNECT;
	}
	else if(listen(s, 5) < 0)
	{
		perror("listen");
		error = ERR_LISTEN;
	}
	else
	{
		// create additional semaphores and initiate our store db
		store->pid = getpid();
		store->dbs = NULL;
		
		// *** import our file system data ***
		// store->dbs = store_import();

		// daemon
		printf("database running...\n");
		while(! stop_server)
		{
			DEBUG_PRINT("notice: iteration\n");
			error = store_server_act(s, &store);
		}

		printf("stopping server...\n");

		// here we should ideally keep a daemon running to
		// check that our memory-values are the same as our
		// disk values
	}

	DEBUG_PRINT("notice: socket 's' has value '%d'\n", s);
	// if the socket was created
	if(s >= 0)
	{
		// now that we've finished, close our communication channel
		close(s);

		// also remove our socket file
		unlink(SERVER_SOCKET_ADDRESS);
	}

	// now that we're done, remove the shared memory
	if(error != ERR_STORE_SHMCREATE && -1 == shmctl(shmid, IPC_RMID, NULL))
	{
		error = ERR_STORE_SHMCTL;
	}

	printf("server stopped\n");

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