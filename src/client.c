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
	int s = -1; // client socket
	struct sockaddr_un addr;
	int len;
	int shmid = -1;
	store_info *store = NULL;
	key_t shm_key = ftok(".", KEY_ID);
	char *ack_msg = NULL; // stores the ack message
	char *ack_buff = NULL; // stores the response
	int max_db_len = 0;
	int max_key_len = 0;
	int ack_len = 0;
	store_req *req = NULL;
	store_req_info req_inf;
	// initialize our request info structure
	req_inf.mode = store->modes[STORE_MODE_SET_ID];
	req_inf.size = sizeof(store_req_data) + (val_len + 1) * sizeof(char)
	               + (num_dbs * max_db_len) * sizeof(char)


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
		DEBUG_PRINT("mode=%c\n", req_inf.mode);
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
	else if(NULL == (req = (store_req *) malloc(req_inf.size)))
	{
		error = ERR_ALLOC;
		print_perror("calloc");
	}
	else
	{
		// init request data
		req->val_len = strlen(value);
		req->num_dbs = num_dbs;

		// CHECK, MAYBE WRONG SINCE SHOULD BE req+1....???
		req->val = req + sizeof(store_req); // value and dbs right after request
		req->dbs = req + sizeof(store_req) + (val_len + 1) * sizeof(char);

		// correct key and dbs variable's size
		strncpy(req->key, key, max_key_len - 1);
		req->key[max_key_len - 1] = '\0';

		// copy the value (we already have reserved for the needed length)
		strcpy(req->val, value);

		for(i = 0; i < num_dbs; i++)
		{
			strncpy(req->dbs + i * max_db_len, dbs[i],
			        max_db_len - 1);
			*(req->dbs + (i + 1) * max_db_len - 1) = '\0';
		}

		// send the data
		DEBUG_PRINT("writing request\n");
		write(s, &req_inf, sizeof(store_req_info));
		// read(s, ack_buff, ack_len); // is it necessary?
		write(s, req, req_len);
		// read(s, ack_buff, ack_len); // setting mode doesn't read any result
		
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
	free(req);

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	int error = ERR_NONE;
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
	store_req *req = NULL;
	store_req_info req_inf;
	store_res *res = NULL; // response
	store_res_info res_inf; // response
	// initialize our request info structure
	req_inf.mode = store->modes[STORE_MODE_SET_ID];
	req_inf.size = sizeof(store_req_data)
	               + (num_dbs * max_db_len) * sizeof(char)


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
	else if(NULL == (req = (store_req *) malloc(req_inf.size)))
	{
		error = ERR_ALLOC;
		print_perror("calloc");
	}
	else
	{
		DEBUG_PRINT("copy key and dbs\n");

		// initialize our request info structure
		req_inf.mode = store->modes[STORE_MODE_SET_ID];
		req_inf.size = sizeof(store_req_data) + (val_len + 1) * sizeof(char)
		               + (num_dbs * max_db_len) * sizeof(char)

		// init request data
		req->val_len = 0; // we don't know it yet
		req->num_dbs = num_dbs;

		// CHECK, MAYBE WRONG SINCE SHOULD BE req+1....???
		req->val = NULL;
		req->dbs = req + sizeof(store_req);

		// correct key and dbs variable's size
		strncpy(req->key, key, max_key_len - 1);
		req->key[max_key_len - 1] = '\0';

		// we will be using single pointers instead of tables to copy db
		for(i = 0; i < num_dbs; i++)
		{
			strncpy(dbs_corrected + i * max_db_len, dbs[i],
			        max_db_len - 1);
			*(dbs_corrected + (i + 1) * max_db_len - 1) = '\0';
			DEBUG_PRINT("got db \"%s\"\n",
			            dbs_corrected + i * max_db_len);
		}

		// send the data
		DEBUG_PRINT("writing request\n");
		write(s, &req_inf, sizeof(store_req_info));
		// read(s, ack_buff, ack_len); // is it necessary?
		write(s, req, req_len);
		// read(s, ack_buff, ack_len); // setting mode doesn't read any result
		
		DEBUG_PRINT("writing finished\n");
		
		// get the response
		for(i = 0; i < num_dbs && error == ERR_NONE; i++)
		{
			DEBUG_PRINT("\nREAD ITERATION %d\n", i);

			// read the value result
			// we can't read the address because it's not shared and it would
			// give a segfault error
			// for(i = 0; i < num_dbs; i++) entries[i];...
			read(s, &val_len, sizeof(int));
			DEBUG_PRINT("val_len \"%d\" read\n", val_len);
			// write(s, ack_msg, ack_len);
			// DEBUG_PRINT("ack written\n");
			
			DEBUG_PRINT("current value pointer before calloc for %d bytes: %p\n",
			            (val_len + 1) * ((int) sizeof(char)), val);
			val = (char *) calloc((val_len + 1), sizeof(char));
			// DEBUG_PRINT("finished calloc\n");
			// DEBUG_PRINT("value pointer after calloc: %p\n", val);

			// memory alloc
			if(NULL != val)
			{
				DEBUG_PRINT("reading value\n");
				read(s, val, (val_len + 1) * sizeof(char));
				DEBUG_PRINT("val \"%s\" read\n", val);
				// write(s, ack_msg, ack_len);
				// DEBUG_PRINT("ack written\n");
	
				printf("%s: %s=%s\n", req->dbs + i * max_db_len,
				       req->key, val);
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
	free(req);
	
	return error;
}

int store_halt()
{
	int error = ERR_NONE;
	int len;
	int s = -1; // client socket
	struct sockaddr_un addr;
	int shmid = -1;
	key_t shm_key = ftok(".", KEY_ID);
	store_info *store = NULL;
	store_req_info req_inf;

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
		req_inf.mode = store->modes[STORE_MODE_STOP_ID];
		req_inf.size = 0;

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
		write(s, &req_inf, sizeof(store_req_info));

		DEBUG_PRINT("notice: database shutting down\n");
	}

	if(s >= 0)
	{
		// close and unlink our socket
		close(s);
	}

	return error;
}
