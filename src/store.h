/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_STORE__
#define __KEYSTORE_STORE__

// pipe paths
#define STORE_PIPE "keystore_fifo"
#define STORE_PIPE_ACK "keystore_fifo_ack"

// pid file path
#define PID_FILENAME "keystore_pid"

// ack message
#define STORE_PIPE_ACK_MSG "ok"
#define STORE_PIPE_ACK_MSG_LEN 3 // sizeof("ok")

#define STORE_PIPE_MODE_SET  's'
#define STORE_PIPE_MODE_GET  'g'
#define STORE_PIPE_MODE_STOP 'x'

// for pipe info sending
struct current_entry_info {
	char key[MAX_KEY_SIZE];
	size_t val_length;
	char *val;
	int num_dbs;
	char *dbs;
	char mode;
	store_entry *entry;
};

void store_write(struct current_entry_info *info);
void store_read(struct current_entry_info *info);
void store_decide();
int store_init();
int store_set(char key[], char value[], int num_dbs, char *dbs[]);
int store_get(char key[], int num_dbs, char *dbs[]);
int store_halt();

#ifdef __DEBUG__
void print_databases(int num_dbs, char *dbs[]);
void print_existing_databases(store_db *store_dbs);
#endif

#endif
