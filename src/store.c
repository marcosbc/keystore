/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */

#include <stdio.h>
#include <sys/shm.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h> // mkfifo types
#include <sys/stat.h> // mkfifo
#include <unistd.h> // fork
#include <fcntl.h> // O_CREAT, ...
#include "common.h"
#include "store.h"
#include "memory.h"
#include "disk.h"
#include "database.h"

// for our signal handlers
int stop_daemon = 0; // used/modified by this process
store_info *store; // used/modified by this process
struct current_entry_info current; // modified by store_set/_get/_halt process
int fifo_pipe;
int error = 0;

void store_write(struct current_entry_info *info)
{
	int err = 0;
	int i = 0;
	pid_t pid;
	int therr = 0;
	store_entry **entries = NULL;
	pthread_t *thids = NULL;
	struct entry_inf *ent_inf = NULL;
	// FILE **fids = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", value \"%s\", num_dbs %d\n",
	            info->key, info->val, info->num_dbs);

	// the parent process modifies the memory so we can print our results
	// faster and let the other process continue writing to disk
	if((pid = fork()) > 0)
	{
		thids = (pthread_t *) calloc(info->num_dbs, sizeof(pthread_t));
		ent_inf = (struct entry_inf *) calloc(info->num_dbs,
		                                      sizeof(struct entry_inf));
	
		if(thids != NULL && entries != NULL && ent_inf != NULL)
		{
			DEBUG_PRINT("alloc ok \n");

			// parent - alter the database in memory
			for(; i < info->num_dbs && ! therr && err == ERR_NONE; i++)
			{
				// create the entry information for setting
				ent_inf[i].key = info->key;
				ent_inf[i].value = info->val;
				ent_inf[i].db_name = info->dbs[i];
				ent_inf[i].entry = NULL;
				ent_inf[i].dbs = store->dbs;
				ent_inf[i].error = 0;
				
				// create our thread
				therr = pthread_create(&thids[i], NULL, memory_set,
				                       &ent_inf[i]);

				DEBUG_PRINT("notice: [parent] thread#%d %d (\"%s\" => \
\"%s\") to insert in db \"%s\"\n",
				            i, (int) thids[i], info->key, info->val,
							ent_inf[i].db_name);

				if(therr != ERR_NONE) 
				{
					err = ERR_THR;
				}
			}
		}

		// set the number of iterations that went correctly
		info->num_dbs = i;
		therr = 0;
		
		// now, end our threads
		for(i = 0; i < info->num_dbs && therr == ERR_NONE && err == ERR_NONE;
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
				info->entry = ent_inf[i].entry;
				err = ent_inf[i].error;
				DEBUG_PRINT("notice: [parent] ended thread#parent-%d %d\
returned value %d\n",
			                i, (int) thids[i], ent_inf[i].error);
			}
		}

		// unmap our shared memory
		if(err != ERR_STORE_SHMLOAD && -1 == shmdt(store))
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
}

void store_read(struct current_entry_info *info)
{
	int err = 0;
	int i;
	int therr = 0;
	pthread_t *thids = NULL;
	struct entry_inf *ent_inf = NULL;

	DEBUG_PRINT("notice: supplied key \"%s\", num_dbs %d\n",
	            info->key, info->num_dbs);

	// create our thread entries
	thids = (pthread_t *) calloc(info->num_dbs,
	                             sizeof(pthread_t));
	ent_inf = (struct entry_inf *) calloc(info->num_dbs,
		                                      sizeof(struct entry_inf));

	if(thids != NULL && ent_inf != NULL)
	{
		DEBUG_PRINT("alloc ok\n");

		for(i = 0; i < info->num_dbs && ! therr && err == ERR_NONE; i++)
		{
			// create the entry information for setting
			ent_inf[i].key = info->key;
			ent_inf[i].value = info->val;
			ent_inf[i].db_name = info->dbs[i];
			ent_inf[i].entry = NULL;
			ent_inf[i].dbs = store->dbs;
			ent_inf[i].error = 0;

			// create our thread
			therr = pthread_create(&thids[i], NULL, memory_get,
			                       &ent_inf[i]);
	
			DEBUG_PRINT("notice: thread#%d %d (key \"%s\" \
 to search in db \"%s\"\n",
			            i, (int) thids[i], info->key, ent_inf[i].db_name);

			if(therr != 0)
			{
				err = ERR_THR;
			}
		}
	
		// set our number of iterations that went correctly
		info->num_dbs = i;

		// end our threads
		for(i = 0; i < info->num_dbs && err == ERR_NONE; i++)
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
				info->entry = ent_inf[i].entry;
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
	else
	{
		err = ERR_ALLOC;
	}

	free(thids);
	free(ent_inf);
}

void store_decide()
{
	struct current_entry_info current;

	DEBUG_PRINT("entering store_decide()\n");

	// get the current_entry_info variable
	// read the length
	read(fifo_pipe, &current.mode, sizeof(char));
	
	// read the struct
	if(current.mode == STORE_PIPE_MODE_SET || current.mode == STORE_PIPE_MODE_GET)
	{
		// read data
		read(fifo_pipe, current.key, MAX_KEY_SIZE * sizeof(char));
		read(fifo_pipe, &current.num_dbs, sizeof(int));
		read(fifo_pipe, current.dbs, current.num_dbs * MAX_DB_SIZE * sizeof(char));

		// make the data persist in disk

		if(current.mode == STORE_PIPE_MODE_SET)
		{
			read(fifo_pipe, &current.val_length, sizeof(size_t));
			read(fifo_pipe, current.val, current.val_length);

			DEBUG_PRINT("mode: setting value\n");

			// set mode
			store_write(&current); // will modify error variable

			// write(entry location)
		}
		else if(current.mode == STORE_PIPE_MODE_GET)
		{
			DEBUG_PRINT("mode: getting value\n");

			// get mode
			store_read(&current); // will modify error variable

			// write(entry location)
		}
	}
	else
	{
		DEBUG_PRINT("mode: stopping db\n");

		printf("freeing memory...\n");

		// sem_wait()
		// free entries[i].val, entries and dbs
		// -> call mem_clean() ???
		// sem_post()

		// close our semaphores -> ERRORS???
		// error = memory_clear();
		
		// stop mode
		stop_daemon = 1;

		// here we should ideally check that our disk-values are
		// the same as our memory-values
	}
}

int store_init()
{
	int shmid;
	FILE *pid_file = NULL;
	key_t shm_key = ftok(".", KEY_ID);
	struct sigaction act;
	
	// set up our sigaction
	act.sa_handler = store_decide;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);  // to stop our store handler
	sigaddset(&act.sa_mask, SIGUSR1); // custom signal: set
	sigaddset(&act.sa_mask, SIGUSR2); // custom signal: get
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);

	DEBUG_PRINT("notice: database starting\n");

	// create our root db
	shmid = shmget(shm_key, sizeof(struct info), IPC_CREAT | IPC_EXCL | 0666);

	// we shouldn't be able to initialize memory when it has already been done
	if(-1 == shmid)
	{
		error = ERR_STORE_SHMCREATE;
	}
	// create our pipe
	else if(-1 == mkfifo(STORE_PIPE, 0666))
	{
		error = ERR_FIFO_CREATE;
	}
	else if(-1 == (fifo_pipe = open(STORE_PIPE, O_RDONLY | O_NONBLOCK)))
	{
		error = ERR_FIFO_OPEN;
	}
	else
	{
		DEBUG_PRINT("got into real action\n");

		// create additional semaphores and initiate our store db
		memory_init();
		
		// store_db.name = ...
		if((store_info *) -1 == (store = shmat(shmid, NULL, 0)))
		{
			error = ERR_STORE_SHMAT;
		}
		else
		{
			// check if our PID file exist (we can't have two running processes)
			if(NULL != (pid_file = fopen(PID_FILENAME, "r")))
			{
				error = ERR_PID_EXIST;
			}
			// also create our PID file
			else if(NULL == (pid_file = fopen(PID_FILENAME, "w")))
			{
				error = ERR_PID_CREATE;
			}
			else
			{
				store->pid = getpid();
				store->dbs = NULL;

				chmod(PID_FILENAME, 0666);

				// write the pid into the file and close it
				fprintf(pid_file, "%d", (int) store->pid);
				fclose(pid_file);

				DEBUG_PRINT("wrote pid %d to pid file\n", (int) store->pid);

				// *** import our file system data ***

				// *** daemon ***
				DEBUG_PRINT("notice: database running...\n");
				while(! stop_daemon)
				{
					// wait for signals until SIGINT received (stop...)
					pause();
				}

				// close and remove the pid file
				unlink(PID_FILENAME);
			}

			// here we should ideally keep a daemon running to
			// check that our memory-values are the same as our
			// disk values
		}

		// close and unlink our pipe
		close(fifo_pipe);
		unlink(STORE_PIPE);
	}

	// now that we're done, remove the shared memory
	if(-1 == shmctl(shmid, IPC_RMID, NULL))
	{
		if(error != ERR_STORE_SHMCREATE)
		{
			error = ERR_STORE_SHMCTL;
		}
	}

	return error;
}

int store_set(char key[], char value[], int num_dbs, char *dbs[])
{
	char mode = STORE_PIPE_MODE_SET;
	int val_len = strlen(value);
	int shmid = 0;
	key_t shm_key = ftok(".", KEY_ID);
	
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
	else if(-1 == (fifo_pipe = open(STORE_PIPE, O_WRONLY | O_NONBLOCK)))
	{
		error = ERR_FIFO_OPEN;
	}
	else
	{
		#ifdef __DEBUG__
		print_existing_databases(store->dbs);
		#endif

		// correct key and dbs variable's size

		// send the signal to our process, and then send the data
		kill(store->pid, SIGUSR1);
		write(fifo_pipe, &mode, sizeof(char));
		write(fifo_pipe, key, MAX_KEY_SIZE * sizeof(char));
		write(fifo_pipe, &num_dbs, sizeof(int));
		write(fifo_pipe, dbs, num_dbs * MAX_DB_SIZE * sizeof(char));
		write(fifo_pipe, &val_len, sizeof(int));
		write(fifo_pipe, value, val_len * sizeof(char));

		// read entry location

		// close our pipe
		close(fifo_pipe);
	}

	return error;
}

int store_get(char key[], int num_dbs, char *dbs[])
{
	char mode = STORE_PIPE_MODE_GET;
	int shmid;
	key_t shm_key = ftok(".", KEY_ID);
	
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
		#ifdef __DEBUG__
		print_existing_databases(store->dbs);
		#endif

		// correct key and dbs variable's size

		// send the signal to our process, and then send the data
		kill(store->pid, SIGUSR2);
		write(fifo_pipe, &mode, sizeof(char));
		write(fifo_pipe, key, MAX_KEY_SIZE * sizeof(char));
		write(fifo_pipe, &num_dbs, sizeof(int));
		write(fifo_pipe, dbs, num_dbs * MAX_DB_SIZE * sizeof(char));

		// read entry location

		// close our pipe
		close(fifo_pipe);
	}

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
    printf("-----------------\n\n");

}
#endif
