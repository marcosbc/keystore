#include <sys/shm.h>
#include <sys/socket.h> // socket related things
#include <sys/un.h> // socket related things
#include <sys/stat.h> // mkfifo
#include <pthread.h>
#include <fcntl.h> // O_CREAT, ...
#include <signal.h>
#include "common.h"
#include "daemon.h"
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

	if(NULL == (*entries = (store_entry **) malloc(num_dbs
	                                                  * sizeof(store_entry *)))
	|| NULL == (ent_inf = (struct entry_inf *) malloc(num_dbs
	                                               * sizeof(struct entry_inf)))
	|| NULL == (thids = (pthread_t *) malloc(num_dbs * sizeof(pthread_t))))
	{
		print_perror("malloc");
		err = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("notice: creating %d threads, to insert \"%s\"=\"%s\"...\n",
		            num_dbs, key, val);
		for(; i < num_dbs && ! therr; i++)
		{
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

			DEBUG_PRINT("notice: thread#%d %d (\"%s\"=\"%s\"), insert in %s\n",
			            i, (int) thids[i], key, val, ent_inf[i].db_name);

			if(therr != ERR_NONE) 
			{
				print_perror("pthread_create");
				err = ERR_THR;
			}
		}

		// set the number of iterations that went correctly
		num_dbs = i;
		therr = 0;
		
		// now, end our threads
		for(i = 0; i < num_dbs && ! therr; i++)
		{
			DEBUG_PRINT("notice: ending thread#%d %d...\n",
			            i, (int) thids[i]);
				
			therr = pthread_join(thids[i], NULL);

			if(therr != 0)
			{
				DEBUG_PRINT("notice: thread#%d %d ended with error %d\n",
				            i, (int) thids[i], therr);
				print_perror("pthread_join");
				err = ERR_THRJOIN;
			}
			else
			{
				(*entries)[i] = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: ended thread#%d %d, returned error %d\n",
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
		DEBUG_PRINT("notice: dbs is NULL\n");
	}
	else if(NULL == (*entries = (store_entry **) malloc(num_dbs
	                                                  * sizeof(store_entry *)))
	     || NULL == (ent_inf = (struct entry_inf *) malloc(num_dbs
	                                               * sizeof(struct entry_inf)))
	     || NULL == (thids = (pthread_t *) malloc(num_dbs * sizeof(pthread_t))))
	{
		print_perror("malloc");
		err = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("notice: creating %d threads, to retrieve \"%s\"...\n",
		            num_dbs, key);
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
			DEBUG_PRINT("notice: thread#%d %d (key \"%s\"), find in db %s\n",
			            i, (int) thids[i], key, ent_inf[i].db_name);

			if(therr != 0)
			{
				print_perror("pthread_create");
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
				DEBUG_PRINT("notice: thread#%d %d ended with error %d\n",
				            i, (int) thids[i], therr);
				print_perror("pthread_join");
				err = ERR_THRJOIN;
			}
			else
			{
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: ended thread#%d %d", i, (int) thids[i]);

				if(err != ERR_NONE || ent_inf[i].entry == NULL)
				{
					(*entries)[i] = NULL;
					DEBUG_PRINT("notice: %s: found *NOTHING* for \"%s\"\n",
					            db_names + i*MAX_DB_SIZE, key);
				}
				else
				{
					(*entries)[i] = ent_inf[i].entry;
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
	struct timeval start, end;
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
		printf(MSG_CONN_INCOMING);

		// init our start time
		gettimeofday(&start, NULL);
		DEBUG_PRINT("notice: request timer started\n");

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
				print_perror("malloc");
				error = ERR_ALLOC;
				res_inf.error = error;
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
						res_inf.error = ERR_INVALID_MODE;
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
					res_inf.error = ERR_INVALID_MODE;
			}

			// send the result back to the client
			write(client_s, &res_inf, sizeof(struct response_info));
		}

		// close connection
		close(client_s);

		// init our start time
		gettimeofday(&end, NULL);
		printf(MSG_REQUEST_TIME_ELAPSED, time_diff(start, end));
	}

	free(result);
	free(res);
	free(req);

	return error;
}

int store_server_init()
{
	int error = 0;
	struct timeval start, running, end;

	// for shared memory operations
	int shmid = -1;
	key_t shm_key = ftok(".", KEY_ID);
	store_db *dbs = NULL;
	store_info *store = NULL; // shared memory to store public configuration
	struct shmid_ds shmbuf; // for changing shm permissions

	// socket related
	int s = -1; // our socket id
	char sock_path[MAX_SOCK_PATH_SIZE];
	struct sockaddr_un addr;
	int len;

	// deal with signals
	struct sigaction act;

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
	if(-1 != (shmid = shmget(shm_key, sizeof(struct info), 0600))
	   || (store_info *) -1 != (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_SESSION;
	}
	// we shouldn't be able to initialize memory when it has already been done
	else if(-1 == (shmid = shmget(shm_key, sizeof(struct info),
	                         IPC_CREAT | IPC_EXCL | 0600)))
	{
		error = ERR_SHMCREATE;
		print_perror("shmget");
	}
	else if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
	{
		error = ERR_SHMAT;
		print_perror("shmat");
	}
	else if(! memory_init())
	{
		error = ERR_SEMOPEN;
		print_perror("sem_open");
	}
	else
	{
		// init our start time
		gettimeofday(&start, NULL);
		DEBUG_PRINT("notice: request timer started\n");

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

		// get the current shared memory settings, to change permissions
		if(-1 == shmctl(shmid, IPC_STAT, &shmbuf))
		{
			print_perror("shmctl: IPC_STAT");
			error = ERR_SHMCTL;
		}
		else
		{
			// write permissions to server, read permissions to the rest
			shmbuf.shm_perm.mode = 0644;
		}
	}

	// if we were able to store the info successfully, proceed
	if(error == ERR_NONE)
	{
		// add global read permissions to shared memory
		if(-1 == shmctl(shmid, IPC_SET, &shmbuf))
		{
			print_perror("shmctl: IPC_SET");
			error = ERR_SHMCTL;
		}
		// create our socket
		else if(-1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
		{
			print_perror("socket");
			error = ERR_SOCKET;
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
			// to get the time it took to start running
			gettimeofday(&running, NULL);
			printf(MSG_RUNNING, time_diff(start, running));

			// daemon
			while(! stop_server)
			{
				DEBUG_PRINT("\n\nnotice: iteration with db %p\n", dbs);
				error = store_server_act(s, &dbs);
			}
			printf(MSG_STOPPING);
		}

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
	if(error != ERR_SHMCREATE && error != ERR_SESSION
	   && error != ERR_SHMAT && -1 == shmdt(store))
	{
		print_perror("shmdt");
		error = ERR_SHMDT;
	}

	// now that we're done, remove the shared memory
	if(error != ERR_SESSION && error != ERR_SHMCREATE
	   && -1 == shmctl(shmid, IPC_RMID, NULL))
	{
		print_perror("shmctl: IPC_RMID");
		error = ERR_SHMCTL;
	}

	if(error != ERR_SEMOPEN && error != ERR_SESSION)
	{
		// clear our semaphores and free memory
		error = memory_clear(&dbs);
	}

	// only show stop message if the server was running before
	if(stop_server != 0)
	{
		gettimeofday(&end, NULL);
		printf(MSG_STOPPED, time_diff(start, end) / 1000);
	}

	return error;
}
