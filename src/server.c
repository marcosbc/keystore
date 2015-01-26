#include <stdio.h>
#include <sys/shm.h>
#include <sys/socket.h> // socket related things
#include <sys/un.h> // socket related things
#include <sys/stat.h> // mkfifo
#include <sys/timeb.h> 
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h> // O_CREAT, ...
#include <signal.h>
#include "common.h"
#include "server.h"
#include "memory.h"
#include "database.h"
#include "sems.h"

volatile int stop_server = 0; // for signals to work

void store_stop()
{
	// inform the user why the database was stopped
	// shmkey = ...
	stop_server = 1;
}

// entries -> pointer of a table of store_entry* variables
// it must be a pointer since we don't have its address 
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
		for(; i < num_dbs && ! therr; i++)
		{
			DEBUG_PRINT("setting key\n");
			// create the entry information for setting
			strncpy(ent_inf[i].key, key, MAX_KEY_SIZE - 1);
			ent_inf[i].key[MAX_KEY_SIZE - 1] = '\0';
			ent_inf[i].value = val;
			ent_inf[i].db_name = db_names + i * MAX_DB_SIZE;
			ent_inf[i].entry = NULL;
			ent_inf[i].dbs = dbs;
			ent_inf[i].error = 0;

			// create our thread
			therr = pthread_create(&thids[i], NULL, memory_set,
			                       &ent_inf[i]);

			DEBUG_PRINT("notice: thread#%d %d (\"%s\" => \
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
			DEBUG_PRINT("notice: ending thread#parent-%d %d...\n",
			            i, (int) thids[i]);
				
			therr = pthread_join(thids[i], NULL);

			if(therr != 0)
			{
				DEBUG_PRINT("notice: thread %d ended with error %d\n",
				            (int) thids[i], therr);
				err = ERR_THRJOIN;
			}
			else
			{
				(*entries)[i] = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: ended thread#parent-%d %d \
returned value %d\n",
			                i, (int) thids[i], ent_inf[i].error);

				if((*entries)[i] == NULL)
				{
					DEBUG_PRINT("notice: %s: found *NOTHING* for \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, key);
				}
				else
				{
					DEBUG_PRINT("notice: %s: found \"%s\": value \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, (*entries)[i]->key,
								(*entries)[i]->val);
				}
			}
		}

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
		DEBUG_PRINT("dbs is NULL\n");
	}
	else if(NULL == (*entries = (store_entry **) calloc(num_dbs,
	                                                    sizeof(store_entry *)))
	     || NULL == (ent_inf = (struct entry_inf *) calloc(num_dbs,
	                                                 sizeof(struct entry_inf)))
	     || NULL == (thids = (pthread_t *) calloc(num_dbs, sizeof(pthread_t))))
	{
		DEBUG_PRINT("err at alloc\n");
		err = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("looping...\n");
		for(i = 0; i < num_dbs && ! therr; i++)
		{
			DEBUG_PRINT("loop %d\n", i);

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
			DEBUG_PRINT("notice: thread#%d %d (key \"%s\") \
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
		for(i = 0; i < num_dbs && ! therr; i++)
		{
			DEBUG_PRINT("notice: ending thread#%d %d...\n",
			            i, (int) thids[i]);

			// finish in order
			therr = pthread_join(thids[i], NULL);

			if(therr != 0)
			{
				DEBUG_PRINT("notice: thread %d ended with error %d\n",
				            (int) thids[i], therr);
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
					DEBUG_PRINT("notice: %s: found *NOTHING* for \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, (*entries)[i]->key);
				}
				else
				{
					DEBUG_PRINT("notice: %s: found \"%s\": val \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, (*entries)[i]->key,
								(*entries)[i]->val);
				}
			}
		}
	}

	free(thids);
	free(ent_inf);

	return err;
}

int store_server_act(int s, store_db **dbs)
{
	int error = 0;
	struct timeb start_tm, end_tm;
	int i = 0;

	// key, value and db pointers for the request
	char *key_ptr = NULL;
	char *val_ptr = NULL;
	char *dbs_ptr = NULL;

	// array of value-sizes and result value for the response
	int *res_size_ptr = NULL;
	char *res_val_ptr = NULL;

	// variables used for TCP communication
	int client_s = -1; // remote socket
	struct sockaddr_un client;
	socklen_t client_len = sizeof(client);

	// requests and responses
	struct request *req = NULL;
	struct request_info req_inf;
	struct response *res = NULL;
	struct response_info res_inf = {
		.size = 0,
		.error = ERR_NONE
	};

	// result of the request query
	store_entry **result = NULL;

	DEBUG_PRINT("notice: waiting for a connection...\n");

	if(-1 != (client_s = accept(s, (struct sockaddr *) &client, &client_len)))
	{
		printf("connection incoming\n");

		// init our start time
		ftime(&start_tm);
		DEBUG_PRINT("has t_start\n");
	
		#ifdef __DEBUG__
		read_lock();
		if(*dbs != NULL)
		{
			print_existing_databases(*dbs);
		}
		read_unlock();
		#endif

		// read data from the client
		read(client_s, &req_inf, sizeof(struct request_info));

		if(req_inf.size != 0)
		{
			if(NULL == (req = (struct request *) malloc(req_inf.size)))
			{
				error = ERR_ALLOC;
				res_inf.error = error;
				print_perror("malloc");
			}
			else
			{
				read(client_s, req, req_inf.size);

				// recalculate pointers with variable
				key_ptr = (char *) (req + 1);
				val_ptr = (char *) (key_ptr + MAX_KEY_SIZE);
				dbs_ptr = (char *) (val_ptr + req->val_size);

				switch(req_inf.mode)
				{
					case STORE_MODE_SET:
						/* error = */ store_write(key_ptr, val_ptr,
						                          req->num_dbs, dbs_ptr,
						                          dbs, &result);
						break;
					case STORE_MODE_GET:
						/* error = */ store_read(key_ptr, req->num_dbs,
						                         dbs_ptr, *dbs, &result);

						// calculate the size of the response
						for(i = 0; i < req->num_dbs; i++)
						{
							if(result != NULL && result[i] != NULL)
							{
								res_inf.size += strlen(result[i]->val);
							}
							res_inf.size += 1;
						}

						// size of struct and each val_len
						res_inf.size += (size_t) req->num_dbs
						                         * sizeof(req->val_size)
						                + sizeof(struct response_info);

						// now, allocate it
						if((res = (struct response *) malloc(res_inf.size))
						   == NULL)
						{
							perror("malloc");
							res_inf.size = 0;
							res_inf.error = ERR_ALLOC;
						}
						else
						{
							// send the result to the client
							res->num = i;
							res_size_ptr = (int *) (res + 1);
							res_val_ptr = (char *) (res_size_ptr + res->num);

							// copy the result to the response variable
							for(i = 0; i < req->num_dbs; i++)
							{
								*res_size_ptr = 1; // size of EOL char is 1
								*res_val_ptr = '\0'; // initialize value to ""

								// did we find anything?
								if(result != NULL && result[i] != NULL)
								{
									strcpy(res_val_ptr, result[i]->val);
									*res_size_ptr += strlen(res_val_ptr);
								}

								// jump to the next element
								res_val_ptr += *res_size_ptr;
								res_size_ptr++;
							}
						}
						break;
					default:
						res_inf.error = 1; // = ERR_INVALID_MODE
				}

				write(client_s, &res_inf, sizeof(struct response_info));
			}

			// in case we have a result to return
			if(res_inf.size > 0)
			{
				write(client_s, res, res_inf.size);
			}
		}
		else
		{
			switch(req_inf.mode)
			{
				case STORE_MODE_STOP:
					store_stop(); // no error
					break;
				default:
					res_inf.error = 1; // = ERR_INCORRECT_MODE
			}

			// send the result back to the client
			write(client_s, &res_inf, sizeof(struct response_info));
		}

		// close connection
		close(client_s);

		// init our start time
		ftime(&end_tm);
		printf("connection closed after %.2f ms\n",
		       (float) 1000.0 * (end_tm.time - start_tm.time)
			           + (end_tm.millitm - start_tm.millitm));
	}

	free(result);
	free(res);
	free(req);

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

	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	DEBUG_PRINT("notice: database starting\n");

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
	if(error == ERR_NONE)
	{
		if(NULL != fopen(sock_path, "r"))
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
