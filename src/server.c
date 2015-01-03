/*
 * PROJECT: Development of a key-value database (includes client)
 * AUTHORS: Marcos Bjorkelund and Adrian Marcelo Anillo
 * NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
 *          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
 *          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
 *          IT GETS RELEASED PUBLICLY.
 */

#include <stdio.h>
#include <sys/shm.h>
#include <sys/socket.h> // socket related things
#include <sys/un.h> // socket related things
#include <sys/stat.h> // mkfifo
#include <sys/timeb.h> 
#include <unistd.h> // fork
#include <pthread.h>
#include <fcntl.h> // O_CREAT, ...
#include <signal.h>
#include "common.h"
#include "server.h"
#include "memory.h"
#include "disk.h"
#include "database.h"
#include "sems.h"

int stop_server = 0; // for signals to work

void store_stop()
{
	// inform the user why the database was stopped
	// shmkey = ...
	stop_server = 1;
}

// TODO: add fork
int store_write(char key[MAX_KEY_SIZE], char *val, int num_dbs,
                char *db_names, store_db **dbs, store_entry ***entries)
{
	int err = 0;
	int i = 0;
	int therr = 0;
	pthread_t *thids = NULL;
	struct entry_inf *ent_inf = NULL;
	// FILE **fids = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", value \"%s\", num_dbs %d\n",
	            key, val, num_dbs);

	if(NULL == (*entries = (store_entry **) calloc(num_dbs,
	                                               sizeof(store_entry *)))
	|| NULL == (ent_inf = (struct entry_inf *) calloc(num_dbs,
	                                                  sizeof(struct entry_inf)))
	|| NULL == (thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t))))
	{
		err = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("alloc ok, entries=%p ent_inf=%p thids=%p\n",
		            *entries, ent_inf, thids);

		// parent - alter the database in memory
		for(; i < num_dbs && ! therr; i++)
		{
			DEBUG_PRINT("setting key\n");
			// create the entry information for setting
			strncpy(ent_inf[i].key, key, MAX_KEY_SIZE - 1);
			ent_inf[i].key[MAX_KEY_SIZE - 1] = '\0';
			DEBUG_PRINT("setting value\n");
			ent_inf[i].value = val;
			DEBUG_PRINT("setting dbname\n");
			ent_inf[i].db_name = db_names + i * MAX_DB_SIZE;
			DEBUG_PRINT("setting entry\n");
			ent_inf[i].entry = NULL;
			DEBUG_PRINT("setting dbs\n");
			ent_inf[i].dbs = dbs;
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

		// set the number of iterations that went correctly
		num_dbs = i;
		therr = 0;
		
		// now, end our threads
		for(i = 0; i < num_dbs && ! therr; i++)
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
				(*entries)[i] = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: [parent] ended thread#parent-%d %d \
returned value %d\n",
			                i, (int) thids[i], ent_inf[i].error);

				if((*entries)[i] == NULL)
				{
					DEBUG_PRINT("%s: found *NO* entry for \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, key);
				}
				else
				{
					DEBUG_PRINT("%s: found entry for \"%s\": value \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, (*entries)[i]->key,
								(*entries)[i]->val);
				}
			}
		}

		DEBUG_PRINT("freeing ent_inf=%p thids=%p\n", ent_inf, thids);
		free(ent_inf);
		free(thids);
	}

	return err;
}

int store_read(char key[MAX_KEY_SIZE], int num_dbs, char *db_names,
               store_db *dbs, store_entry ***entries)
{
	int err = 0;
	int i;
	int therr = 0;
	pthread_t *thids = NULL;
	struct entry_inf *ent_inf = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", num_dbs %d\n",
	            key, num_dbs);

	if(dbs == NULL)
	{
		err = ERR_DB;
	}
	else if(NULL == (*entries = (store_entry **) calloc(num_dbs,
	                                                   sizeof(store_entry *)))
	     || NULL == (ent_inf = (struct entry_inf *) calloc(num_dbs,
	                                                sizeof(struct entry_inf)))
	     || NULL == (thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t))))
	{
		err = ERR_ALLOC;
	}
	else
	{
		for(i = 0; i < num_dbs && ! therr; i++)
		{
			// create the entry information for setting
			strncpy(ent_inf[i].key, key, MAX_KEY_SIZE - 1);
			ent_inf[i].value = NULL;
			ent_inf[i].db_name = db_names + i * MAX_DB_SIZE;
			ent_inf[i].entry = NULL;
			ent_inf[i].dbs = &dbs;
			ent_inf[i].error = 0;

			// create our thread -> ERROR???
			therr = pthread_create(&thids[i], NULL, memory_get,
			                       &ent_inf[i]);

			// check val, it's null probably
			DEBUG_PRINT("notice: thread#%d %d (key \"%s\" \
 to search in db \"%s\")\n",
			            i, (int) thids[i], key, ent_inf[i].db_name);

			if(therr != 0)
			{
				err = ERR_THR;
			}
		}
	
		// set our number of iterations that went correctly
		num_dbs = i;

		// end our threads
		for(i = 0; i < num_dbs && ! therr; i++)
		{
			DEBUG_PRINT("notice: ending thread#%d %d...\n",
			            i, (int) thids[i]);

			// finish in order
			therr = pthread_join(thids[i], NULL);

			if(therr != 0)
			{
				DEBUG_PRINT("thread ended, error %d\n", therr);
				err = ERR_THRJOIN;
			}
			else
			{
				(*entries)[i] = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: ended thread#%d %d, returned %d\n",
				            i, (int) thids[i], err);

				if((*entries)[i] == NULL)
				{
					DEBUG_PRINT("%s: found *NO* entry for \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, (*entries)[i]->key);
				}
				else
				{
					DEBUG_PRINT("%s: found entry for \"%s\": value \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, (*entries)[i]->key,
								(*entries)[i]->val);
				}
			}
		}
	}

	DEBUG_PRINT("freeing ent_inf=%p thids=%p\n", ent_inf, thids);
	free(thids);
	free(ent_inf);

	return err;
}

int store_server_act(int s, store_db **dbs)
{
	struct timeb start_tm, end_tm;
	int error = 0;
	int client_s = -1; // remote socket
	struct sockaddr_un client;
	socklen_t client_len = sizeof(client);
	char mode;
	char key[MAX_KEY_SIZE];
	int num_dbs; // 32-bit and 64-bit intercompatibility
	char *db_names = NULL;
	int val_len = 0; // 32-bit and 64-bit intercompatibility
	char *val = NULL;
	store_entry **result = (store_entry **) NULL;
	int i = 0;
	char ack_buff[STORE_ACK_LEN];
	
	DEBUG_PRINT("waiting for a connection...\n");

	// accept connection (if it fails, it is because server was closed: no error)
	if(-1 != (client_s = accept(s, (struct sockaddr *) &client, &client_len)))
	{
		printf("connection incoming\n");

		// init our start time
		ftime(&start_tm);
		DEBUG_PRINT("has t_start\n");
	
		#ifdef __DEBUG__
		if(*dbs != NULL)
		{
			print_existing_databases(*dbs);
		}
		#endif

		// read data
		read(client_s, &mode, sizeof(char));
		write(client_s, STORE_ACK, STORE_ACK_LEN);
		DEBUG_PRINT("got mode \"%c\" from client\n", mode);

		if(mode == STORE_MODE_SET || mode == STORE_MODE_GET)
		{
			DEBUG_PRINT("setting or getting mode\n");
			read(client_s, &key, MAX_KEY_SIZE * sizeof(char));
			write(client_s, STORE_ACK, STORE_ACK_LEN);
			DEBUG_PRINT("got key \"%s\" from client\n", key);

			read(client_s, &num_dbs, sizeof(int));
			write(client_s, STORE_ACK, STORE_ACK_LEN);
			DEBUG_PRINT("got num_dbs \"%d\" from client\n", num_dbs);
			
			db_names = (char *) calloc(num_dbs * MAX_DB_SIZE, sizeof(char));
			if(db_names == NULL)
			{
				print_perror("calloc");
				error = ERR_ALLOC;
			}
			else
			{
				read(client_s, db_names, num_dbs * MAX_DB_SIZE * sizeof(char));
				for(i = 0; i < num_dbs; i++)
				{
					DEBUG_PRINT("got dbs[%d] \"%s\" from client\n", i,
					            db_names + i*MAX_DB_SIZE);
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
					val = (char *) calloc((val_len + 1), sizeof(char));
			
					if(val == NULL)
					{
						print_perror("calloc");
						error = ERR_ALLOC;
					}
					else
					{
						// ack is not needed here, since we'll return a result
						/* val_len = */read(client_s, val, val_len * sizeof(char));
						val[val_len] = '\0'; // ensure we reach end of string
						DEBUG_PRINT("got value \"%s\" from client\n", val);

						DEBUG_PRINT("proceeding to write\n");
						// now that we have everything, call function for setting
						/* error = */ store_write(key, val, num_dbs, db_names,
						                          dbs, &result);
						
						// and finished, since we don't respond to set reqs
					}
				}
				// 'get' mode
				else
				{
					DEBUG_PRINT("getting mode\n");
					
					DEBUG_PRINT("proceeding to read\n");
					// now that we have everything, call function for setting
					/* error = */ store_read(key, num_dbs, db_names, *dbs,
					                         &result);

					DEBUG_PRINT("got result %p\n", &result);

					// send the result
					for(i = 0; i < num_dbs; i++)
					{
						DEBUG_PRINT("\nWRITE ITERATION %d\n", i);

						// if we found an entry, write it
						if(result != NULL && result[i] != NULL)
						{
							DEBUG_PRINT("going for iteration\n");
							val_len = (int) strlen(result[i]->val);
							DEBUG_PRINT("writing val len %d\n", val_len);
							write(client_s, &val_len, sizeof(int));
							read(client_s, &ack_buff, STORE_ACK_LEN);
							DEBUG_PRINT("read ack \"%s\"\n", ack_buff);
							
							write(client_s, result[i]->val,
							                (val_len + 1) * sizeof(char));
							DEBUG_PRINT("result \"%s\" written\n", result[i]->val);
							read(client_s, &ack_buff, STORE_ACK_LEN);
							DEBUG_PRINT("read ack \"%s\"\n", ack_buff);
						}
						// we want to send a response even if the "result"
						// variable is null
						else
						{
							val_len = 0;
							DEBUG_PRINT("val_len is ZERO, not found\n");
							write(client_s, &val_len, sizeof(int));
							read(client_s, &ack_buff, STORE_ACK_LEN);
							DEBUG_PRINT("read ack \"%s\"\n", ack_buff);

							write(client_s, "", (val_len + 1) * sizeof(char));
							DEBUG_PRINT("result \"\" written\n");
							read(client_s, &ack_buff, STORE_ACK_LEN);
							DEBUG_PRINT("read ack \"%s\"\n", ack_buff);
						}

						DEBUG_PRINT("---DONE---\n\n");
					}
				}
				DEBUG_PRINT("clearing result %p\n", result);
				free(result); // it is an array, so we can free it
				result = NULL;
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
		ftime(&end_tm);
		printf("connection closed after %.2f ms\n",
		       (float) 1000.0 * (end_tm.time - start_tm.time)
			           + (end_tm.millitm - start_tm.millitm));
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
	int len;
	store_db *dbs = NULL;
	store_info *store = NULL; // shared memory to store public configuration
	char sock_path[MAX_SOCK_PATH_SIZE];

	// set up signal to stop server correctly
	act.sa_handler = store_stop;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);

	DEBUG_PRINT("notice: database starting\n");

	// ignore our sigpipe signal
	signal(SIGPIPE, SIG_IGN);

	// why or? so we make sure there are no conflicts
	if(-1 != (shmid = shmget(shm_key, sizeof(struct info), 0664))
	   || (store_info *) -1 != (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_SESSION;
		print_error(ERR_SESSION_MSG);
		DEBUG_PRINT("shmid=%d, store=%p (-1 = %p)\n", shmid,
		            store, (store_info *) -1);
	}
	// we shouldn't be able to initialize memory when it has already been done
	else if(-1 == (shmid = shmget(shm_key, sizeof(struct info),
	                         IPC_CREAT | IPC_EXCL | 0664)))
	{
		error = ERR_STORE_SHMCREATE;
		print_perror("shmget");
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_STORE_SHMAT;
		print_perror("shmat");
	}
	else if(! memory_init())
	{
		error = ERR_MEM_SEMOPEN;
		print_perror("sem_open");
	}
	else
	{
		write_lock();

		// init our socket info, everything went ok
		strcpy(store->sock_path, STORE_SOCKET_PATH);

		// make a clone of the sock_path var
		strcpy(sock_path, store->sock_path);

		// set sockaddr information values
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, store->sock_path);
		len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);
	
		// unlink our socket so we don't get errors if it is already created
		unlink(store->sock_path);

		// set public configuration info in our shared memory
		store->pid = getpid();
		store->ack_len = STORE_ACK_LEN;
		strcpy(store->ack_msg, STORE_ACK);
		store->max_sock_len = MAX_SOCK_PATH_SIZE;
		strcpy(store->sock_path, STORE_SOCKET_PATH);
		store->max_key_len = MAX_KEY_SIZE;
		store->max_val_len = MAX_VAL_SIZE;
		store->max_db_len = MAX_DB_SIZE;
		store->modes[STORE_MODE_SET_ID] = STORE_MODE_SET;
		store->modes[STORE_MODE_GET_ID] = STORE_MODE_GET;
		store->modes[STORE_MODE_STOP_ID] = STORE_MODE_STOP;
		write_unlock();
	}

	// if we were able to store the info successfully, proceed
	if(error == ERR_NONE && NULL != fopen(sock_path, "r"))
	{
		print_error("socket connection already exists");
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
		print_perror("bind");
		error = ERR_BIND;
	}
	else if(listen(s, 5) < 0)
	{
		print_perror("listen");
		error = ERR_LISTEN;
	}
	else
	{
		// *** import our file system data ***
		// dbs = store_import();

		// daemon
		printf("database running...\n");
		while(! stop_server)
		{
			DEBUG_PRINT("\n\nnotice: iteration with db %p\n", dbs);
			error = store_server_act(s, &dbs);
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
		unlink(sock_path);
	}

	// unmap our shared memory
	if(error != ERR_STORE_SHMCREATE && error != ERR_SESSION
	   && error != ERR_STORE_SHMAT && -1 == shmdt(store))
	{
		error = ERR_STORE_SHMDT;
	}

	// now that we're done, remove the shared memory
	if(error != ERR_SESSION && error != ERR_STORE_SHMCREATE
	   && -1 == shmctl(shmid, IPC_RMID, NULL))
	{
		error = ERR_STORE_SHMCTL;
	}

	if(error != ERR_MEM_SEMOPEN && error != ERR_SESSION)
	{
		// clear our semaphores and free memory
		error = memory_clear(&dbs);
	}

	printf("server stopped\n");

	return error;
}
