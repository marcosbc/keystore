#include <stdio.h>
#include <sys/socket.h> // socket related things
#include <sys/un.h> // socket related things
#include <sys/stat.h> // mkfifo
#include <sys/shm.h> // 
#include <unistd.h>
#include <fcntl.h> // O_CREAT, ...
#include <pthread.h>
#include "common.h"
#include "client.h"
#include "database.h"
#include "sems.h"

int store_set(char key[], char *value, int num_dbs, char *dbs[])
{
	int error = ERR_NONE;
	int i;

	// variables for communication via TCP
	int s = -1;
	struct sockaddr_un addr;

	// shared memory and common settings
	int shmid = -1;
	store_info *store = NULL;
	key_t shm_key = ftok(".", KEY_ID);
	int max_db_len = 0;
	int max_key_len = 0;

	// result and request
	struct request *req = NULL;
	struct request_info req_inf = {
		.mode = store->modes[STORE_MODE_SET_ID];
		.size = sizeof(struct request) + (val_len + 1) * sizeof(char)
		        + (num_dbs * max_db_len) * sizeof(char);
	};

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
	else
	{
		DEBUG_PRINT("get first set of values from shm\n");

		// set our mode and other variables
		max_db_len = store->max_db_len;
		max_key_len = store->max_key_len;

		// set sockaddr information values
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, store->sock_path);
		len = sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1);

		DEBUG_PRINT("got server config pid=%d, socklen=%d \
sockpath=%s keylen=%d vallen=%d dblen=%d\n",
					 (int) store->pid,
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
	else if(connect(s, (struct sockaddr *) &addr,
	                sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1)) == -1)
	{
		print_perror("connect");
		error = ERR_CONNECT;
	}
	else if(NULL == (req = (struct request *) malloc(req_inf.size)))
	{
		print_perror("calloc");
		error = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("notice: preparing for setting");

		// init request data
		req->val_len = strlen(value);
		req->num_dbs = num_dbs;

		// CHECK, MAYBE WRONG SINCE SHOULD BE req+1....???
		req->val = (char *) (req + 1);
		req->dbs = (char *) (req->val + val_len + 1);

		// correct key
		strncpy(req->key, key, max_key_len - 1);
		req->key[max_key_len - 1] = '\0';

		// copy the value (we already have reserved for the needed length)
		strcpy(req->val, value);

		// copy dbs and correct
		for(i = 0; i < num_dbs; i++)
		{
			strncpy(req->dbs + i * max_db_len, dbs[i],
			        max_db_len - 1);
			*(req->dbs + (i + 1) * max_db_len - 1) = '\0';
		}

		// send the data
		DEBUG_PRINT("notice: writing request\n");
		write(s, req_inf, sizeof(struct request_info));
		write(s, req, &req_inf.size);

		// read the result
		DEBUG_PRINT("notice: getting response\n");
		read(s, &res_inf, sizeof(struct response_info));
		DEBUG_PRINT("read res_info num=%d sz=%d\n", res_inf.num, res_inf.size);

		if(res_inf.size <= 0 || res_inf.num != num_dbs)
		{
			DEBUG_PRINT("notice: malformed response\n");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			// error = ...;
		}
		if(res_inf.error != ERR_NONE)
		{
			DEBUG_PRINT("notice: something went wrong on the server side");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			error = res->error;
		}
		else if(NULL == (res = (struct response *) calloc(1, res_inf.size)))
		{
			print_perror("calloc");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			error = ERR_ALLOC;
		}
		else
		{
			read(s, res, res_inf.size);
			DEBUG_PRINT("read res%p err=%d\n", res);
			DEBUG_PRINT("len=%p val=%p", res->len, res->val);

			DEBUG_PRINT("notice: *** DONE ***\n\n");
		}

	}

	if(s >= 0)
	{
		// close our socket to stop communication
		close(s);
	}

	if(error != ERR_STORE_SHMLOAD && error != ERR_STORE_SHMAT
	   && -1 == shmdt(store))
	{
		print_perror("shmdt");
		error = ERR_STORE_SHMDT;
	}

	free(req);
	free(res);

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	int error = ERR_NONE;
	int i = 0;

	// variables for communication via TCP
	int s = -1; // client socket
	struct sockaddr_un addr;

	// shared memory, common settings variables
	store_info *store = NULL;
	key_t shm_key = ftok(".", KEY_ID);
	int shmid = -1;
	int max_db_len;
	int max_key_len;

	// response and request
	struct response *res = NULL;
	struct response_info res_inf; // response
	struct request *req = NULL;
	struct request_info req_inf = {
		.mode = store->modes[STORE_MODE_STOP_ID];
		.size = sizeof(struct request) + (num_dbs * max_db_len) * sizeof(char);
	};

	// initialize our semaphores and begin the semaphore lock
	if(! sems_open())
	{
		error = ERR_MEM_SEMOPEN;
	}
	// get our server's public config information
	else if(-1 == (shmid = shmget(shm_key, sizeof(struct info), 0664)))
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

		// set our mode and other variables
		mode = store->modes[STORE_MODE_GET_ID];
		max_db_len = store->max_db_len;
		max_key_len = store->max_key_len;

		// set sockaddr information values
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, store->sock_path);
		
		read_unlock();
	}

	// initialize our socket
	if(error == ERR_NONE && -1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		DEBUG_PRINT("error at socket creation\n");
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr,
	                sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1)) == -1)
	{
		print_perror("connect");
		error = ERR_CONNECT;
	}
	else if(NULL == (req = (struct request *) malloc(req_inf.size)))
	{
		print_perror("calloc");
		error = ERR_ALLOC;
	}
	else
	{
		DEBUG_PRINT("notice: preparing for getting");

		// init request data
		req->val_len = 0;
		req->num_dbs = num_dbs;

		// CHECK, MAYBE WRONG SINCE SHOULD BE req+1....???
		req->val = NULL;
		req->dbs = (char *) (req + 1);

		// correct key
		strncpy(req->key, key, max_key_len - 1);
		req->key[max_key_len - 1] = '\0';

		// copy dbs and correct
		for(i = 0; i < num_dbs; i++)
		{
			strncpy(req->dbs + i * max_db_len, dbs[i],
			        max_db_len - 1);
			*(req->dbs + (i + 1) * max_db_len - 1) = '\0';
		}

		// send the data
		DEBUG_PRINT("notice: sending request\n");
		write(s, &req_inf, sizeof(struct request_info));
		write(s, req, &req_inf.size);

		// read the result
		DEBUG_PRINT("notice: getting response\n");
		read(s, &res_inf, sizeof(struct response_info));
		DEBUG_PRINT("read res_info num=%d sz=%d\n", res_inf.num, res_inf.size);

		if(res_inf.size <= 0 || res_inf.num != num_dbs)
		{
			DEBUG_PRINT("notice: malformed response\n");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			// error = ...;
		}
		if(res_inf.error != ERR_NONE)
		{
			DEBUG_PRINT("notice: something went wrong on the server side");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			error = res->error;
		}
		else if(NULL == (res = (struct response *) calloc(1, res_inf.size)))
		{
			print_perror("calloc");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			error = ERR_ALLOC;
		}
		else
		{
			read(s, res, res_inf.size);
			DEBUG_PRINT("read res%p err=%d\n", res);
			DEBUG_PRINT("res err=%d len=%p val=%p", res->error, res->len, res->val);

			if(NULL == res->val)
			{
				DEBUG_PRINT("notice: result's val is NULL\n");
				DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			}
			else
			{
				// now we checked everything is ok, print result
				for(i = 0; i < num_dbs; i++)
				{
					printf("%s: %s=%s\n", req->dbs + i * max_db_len,
					                      key,
										  res->val + i * val_len);
				}

				DEBUG_PRINT("notice: *** DONE ***\n\n");
			}
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
		print_perror("shmdt");
		error = ERR_STORE_SHMDT;
	}

	free(req);
	free(res);
	
	return error;
}

int store_halt()
{
	int error = ERR_NONE;

	// variables for communication via TCP
	int s = -1; // client socket
	struct sockaddr_un addr;

	// shared memory and common settings (in this case, only socket path)
	int shmid = -1;
	key_t shm_key = ftok(".", KEY_ID);
	store_info *store = NULL;

	// result and request `headers'
	struct response_info res_inf; // response
	struct request_info req_inf = {
		.mode = store->modes[STORE_MODE_STOP_ID];
		.size = 0;
	};

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
		
		// set sockaddr information values
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, store->sock_path);
		
		read_unlock();
	}
	
	DEBUG_PRINT("notice: database preparing to shut down\n");

	// initialize our socket
	if(error == ERR_NONE && -1 >= (s = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		error = ERR_SOCKETCREATE;
	}
	// connect to socket
	else if(connect(s, (struct sockaddr *) &addr,
	                sizeof(addr.sun_family) + (strlen(addr.sun_path) + 1)) == -1)
	{
		print_perror("connect");
		error = ERR_CONNECT;
	}
	else
	{
		// send halt character to daemon (for shutdown)
		write(s, &req_inf, sizeof(struct request_info));

		DEBUG_PRINT("notice: database shutting down\n");

		read(s, &res_inf, sizeof(struct response_info));

		if(res_inf.size != 0 || res_inf.num != 0)
		{
			DEBUG_PRINT("notice: malformed response");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			// error = ...;
		}
		else if(res_inf.error != ERR_NONE)
		{
			DEBUG_PRINT("server side error occured");
			DEBUG_PRINT("notice: *** NOT DONE, ERROR HAPPENED ***\n");
			error = res_inf.error;
		}
		else
		{
			DEBUG_PRINT("notice: *** DONE ***\n\n");
		}
	}

	if(s >= 0)
	{
		// close and unlink our socket
		close(s);
	}

	return error;
}

// int send_data(int s, int val_len, char *val);
