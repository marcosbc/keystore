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
#include <sys/stat.h> // mkfifo
#include <sys/times.h>
#include <time.h>
#include <unistd.h> // fork
#include <pthread.h>
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
				DEBUG_PRINT("setting key\n");
				// create the entry information for setting
				strncpy(ent_inf[i].key, key, MAX_KEY_SIZE - 1);
				ent_inf[i].key[MAX_KEY_SIZE - 1] = '\0';
				DEBUG_PRINT("setting value\n");
				ent_inf[i].value = val;
				DEBUG_PRINT("setting dbname\n");
				ent_inf[i].db_name = dbs + i * MAX_DB_SIZE;
				DEBUG_PRINT("setting entry\n");
				ent_inf[i].entry = NULL;
				DEBUG_PRINT("setting store->dbs\n");
				ent_inf[i].dbs = &((*store)->dbs);
				DEBUG_PRINT("setting error\n");
				ent_inf[i].error = 0;
				DEBUG_PRINT("creating thread\n");
				
				// create our thread
				therr = pthread_create(&thids[i], NULL, memory_set,
				                       &ent_inf[i]);

				DEBUG_PRINT("notice: [parent] thread#%d %d (\"%s\" => \
\"%s\") to insert in db \"%s\"\n",
				            i, (int) thids[i], key, val,
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
				
			therr = pthread_join(thids[i], NULL);

			if(therr != 0)
			{
				err = ERR_THRJOIN;
			}
			else
			{
				entry = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: [parent] ended thread#parent-%d %d \
returned value %d\n",
			                i, (int) thids[i], ent_inf[i].error);
			}
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
			strncpy(ent_inf[i].key, key, MAX_KEY_SIZE - 1);
			ent_inf[i].key[MAX_KEY_SIZE - 1] = '\0';
			ent_inf[i].value = NULL;
			ent_inf[i].db_name = dbs + i * MAX_DB_SIZE;
			ent_inf[i].entry = NULL;
			ent_inf[i].dbs = &(store->dbs);
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
				DEBUG_PRINT("notice: ended thread#%d %d, returned %d\n",
				            i, (int) thids[i], err);
			}
		}
	}

	free(thids);
	free(ent_inf);

	return entry;
}

int store_server_act(int s, store_info **store)
{
	long ticks_per_sec = sysconf(_SC_CLK_TCK);
	struct tms start_tm, end_tm;
	clock_t t_start, t_end;
	int error = 0;
	int client_s = -1; // remote socket
	struct sockaddr_un client;
	socklen_t client_len = sizeof(client);
	char mode;
	char key[MAX_VAL_SIZE];
	int num_dbs; // 32-bit and 64-bit intercompatibility
	char *dbs = NULL;
	int val_len = 0; // 32-bit and 64-bit intercompatibility
	char *val = NULL;
	store_entry *result = (store_entry *) -1;
	int i = 0;// debug
	char ack_buff[STORE_ACK_LEN];
	
	DEBUG_PRINT("waiting for a connection...\n");

	// accept connection (if it fails, it is because server was closed: no error)
	if(-1 != (client_s = accept(s, (struct sockaddr *) &client, &client_len)))
	{
		DEBUG_PRINT("accepted socket\n");

		// init our start time
		t_start = times(&start_tm);
		DEBUG_PRINT("has t_start\n");
	
		#ifdef __DEBUG__
		if((*store)->dbs != NULL)
		{
			print_existing_databases((*store)->dbs);
		}
		#endif

		// read data
		read(client_s, &mode, sizeof(char));
		write(client_s, STORE_ACK, STORE_ACK_LEN);
		DEBUG_PRINT("got mode \"%c\" from client\n", mode);

		if(mode == STORE_MODE_SET || mode == STORE_MODE_GET)
		{
			DEBUG_PRINT("setting or getting mode\n");
			read(client_s, &key, MAX_VAL_SIZE * sizeof(char));
			write(client_s, STORE_ACK, STORE_ACK_LEN);
			DEBUG_PRINT("got key \"%s\" from client\n", key);

			read(client_s, &num_dbs, sizeof(int));
			write(client_s, STORE_ACK, STORE_ACK_LEN);
			DEBUG_PRINT("got num_dbs \"%d\" from client\n", num_dbs);
			
			dbs = (char *) malloc(num_dbs * MAX_DB_SIZE * sizeof(char));
			if(dbs == NULL)
			{
				perror("malloc");
				error = ERR_ALLOC;
			}
			else
			{
				read(client_s, dbs, num_dbs * MAX_DB_SIZE * sizeof(char));
				for(i = 0; i < num_dbs; i++)
				{
					DEBUG_PRINT("got dbs[%d] \"%s\" from client\n", i,
					            dbs + i*MAX_DB_SIZE);
				}

				// if we're setting a value, we must also read it
				if(mode == STORE_MODE_SET)
				{
					DEBUG_PRINT("setting mode\n");
					write(client_s, STORE_ACK, STORE_ACK_LEN);

					// first, get our val_len
					read(client_s, &val_len, sizeof(int));
					write(client_s, STORE_ACK, STORE_ACK_LEN);
					DEBUG_PRINT("got val_len \"%d\" from client\n", val_len);

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
						
						// ack is not needed here, since we'll return a result
						/* val_len = */read(client_s, val, val_len * sizeof(char));
						val[val_len] = '\0'; // ensure we reach end of string
						DEBUG_PRINT("got value \"%s\" from client\n", val);

						DEBUG_PRINT("proceeding to write\n");
						// now that we have everything, call function for setting
						result = store_write(key, val, num_dbs, dbs, store);

						// send the result pointer address to our receiver
					}
				}
				// 'get' mode
				else
				{
					DEBUG_PRINT("getting mode\n");
					
					DEBUG_PRINT("proceeding to read\n");
					// now that we have everything, call function for setting
					result = store_read(key, num_dbs, dbs, *store);

					DEBUG_PRINT("got result %p\n", result);

					// send the result
					if(result != NULL)
					{
						val_len = (int) strlen(result->val);
						DEBUG_PRINT("writing val len %d\n", val_len);
						write(client_s, &val_len, sizeof(int));
						read(client_s, &ack_buff, STORE_ACK_LEN);
						DEBUG_PRINT("read ack %s\n", ack_buff);
						write(client_s, result->val, sizeof(char *));
						DEBUG_PRINT("result \"%s\" written\n", result->val);
					}
					// if we didn't find it, return an empty string
					else
					{
						val_len = 0;
						DEBUG_PRINT("val_len is ZERO, not found\n");
						write(client_s, &val_len, sizeof(int));
						read(client_s, &ack_buff, STORE_ACK_LEN);
						DEBUG_PRINT("read ack %s\n", ack_buff);
						write(client_s, "", sizeof(char *));
						DEBUG_PRINT("result \"\" written\n");
					}
				}
			}
		}
		// formal server stop so no error
		else
		{
			store_stop();
		}

		// close connection
		close(client_s);

		// init our start time
		t_end = times(&end_tm);
		printf("connection closed after %7.4f ms\n",
		       (float) 1000 * (t_end - t_start) / ticks_per_sec);
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
	int len;

	// set sockaddr information values
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, STORE_SOCKET_PATH);
	len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);

	// set up signal to stop server correctly
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
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
	}
	else if(! memory_init())
	{
		error = ERR_MEM_SEMOPEN;
		perror("sem_open");
	}
	else if(NULL != fopen(STORE_SOCKET_PATH, "r"))
	{
		fprintf(stderr, "error: socket connection already exists");
		error = ERR_SOCKETEXIST;
	}
	// create our socket
	else if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// bind socket to file
	else if (bind(s, (struct sockaddr *) &addr, len) < 0)
	{
		perror("bind");
		error = ERR_BIND;
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
			DEBUG_PRINT("\n\n\nnotice: iteration with store %p\n", store);
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
		unlink(STORE_SOCKET_PATH);
	}

	// unmap our shared memory
	if(error != ERR_STORE_SHMLOAD && -1 == shmdt(store))
	{
		error = ERR_STORE_SHMDT;
	}

	// now that we're done, remove the shared memory
	if(error != ERR_STORE_SHMCREATE && -1 == shmctl(shmid, IPC_RMID, NULL))
	{
		error = ERR_STORE_SHMCTL;
	}

	printf("server stopped\n");

	return error;
}
