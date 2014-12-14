/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __HTTPSERV_CONFIG__
#define __HTTPSERV_CONFIG__

#define MAX_ENTRIES 8

#define MAX_BUFFER_SIZE 1024

#define HTDIR_KEY "directory"
#define PORT_KEY "port"
#define MODE_KEY "mode"

#define MAX_ERROR_LEN 200
#define ERROR_CONFIG "error parsing config file\n"
#define ERROR_SYNC "error in config file: mode, port and directory need to be correctly set\n"

typedef struct {
	char key[MAX_KEY_SIZE];
	char value[MAX_VALUE_SIZE];
} config_entry;

int process_config(char filepath[], servinfo *conf);
int entry_size(config_entry entries);
void init_config(servinfo *conf);
int extract_config_line(FILE *f, char buffer[MAX_BUFFER_SIZE], config_entry *entry);
int is_valid_config_line(char buffer[MAX_BUFFER_SIZE], config_entry *entry);
int is_empty_line(char line[MAX_BUFFER_SIZE]);
int is_comment_line(char line[MAX_BUFFER_SIZE]);
int sync_entries(config_entry entries[MAX_ENTRIES], servinfo *conf, int num_entries);
int is_entry_empty(config_entry e);
void error_case(char *description, int is_error);

#endif
