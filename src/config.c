/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "config.h"

// returns a well-formatted servinfo variable
int process_config(char filepath[], servinfo *conf)
{
	FILE *conf_file = fopen(filepath, "r");
	char buffer[MAX_BUFFER_SIZE];
	int error_occurred = 1;
	config_entry entries[MAX_ENTRIES];
	int num_entries = 0;

	// to detect possible errors, initialize conf variable
	init_config(conf);

	// we can only proceed if the file got opened
	if(conf_file != NULL) {
		do {
			// extract our config file, error if something went wrong
			error_occurred = ! extract_config_line(conf_file, buffer, &entries[num_entries]);
			error_case(ERROR_CONFIG, error_occurred);
			
			if(! feof(conf_file) && ! error_occurred && num_entries < MAX_ENTRIES)
			{
				num_entries++;
			}
		} while(! error_occurred && ! feof(conf_file) && num_entries < MAX_ENTRIES);

		// now add to conf file
		if(! error_occurred)
		{
			error_occurred = ! sync_entries(entries, conf, num_entries);

			error_case(ERROR_SYNC, error_occurred);
		}
	}
	else
	{
		perror("config file doesn't exist");
	}

	return error_occurred;
}

int entry_size(config_entry entries)
{
	return sizeof(entries) / sizeof(config_entry);
}

void init_config(servinfo *conf)
{
	conf->port = 0;
	strcpy(conf->htdir, "");
	conf->mode = MODE_NONE;
}

int extract_config_line(FILE *f, char buffer[MAX_BUFFER_SIZE], config_entry *entry)
{
	int has_more, valid_line, ignore_line = 1;
	has_more = (NULL != fgets(buffer, MAX_BUFFER_SIZE, f));
	int i;

	// continue until there is a valid non-ignored line
	while(has_more && ignore_line)
	{
		// add_end_on_string_newline(&buffer);
		i = 0;
		while(buffer != NULL && i >= 0)
		{
			if(buffer[i] == '\n')
			{
				buffer[i] = '\0';
				i = -1;
			}
			else
			{
				i++;
			}
		}
		valid_line = is_valid_config_line(buffer, entry);
		ignore_line = valid_line < 0; // empty line or comment convention

		if(! valid_line)
		{
			fprintf(stderr, "error: error at line: %s\n", buffer);
		}
		else if(ignore_line)
		{
			has_more = (NULL != fgets(buffer, MAX_BUFFER_SIZE, f));
		}
	}

	// !0: ok, 0: error
	return valid_line;
}

/* valid lines are:
 * -1 - comments or empty
 *  1 - key=value
 *  0 - not valid
 */
int is_valid_config_line(char buffer[MAX_BUFFER_SIZE], config_entry *entry)
{
	int is_valid = 0;
	char *buffer_copy = (char *) malloc(sizeof(char) * MAX_BUFFER_SIZE);
	char *buffer_copy_pointer = buffer_copy;

	if(NULL != buffer_copy)
	{
		// check if it is a comment or empty line
		if(is_comment_line(buffer) || is_empty_line(buffer))
		{
			is_valid = -1;
		}
		else if(NULL != strstr(buffer, "="))
		{
			strcpy(buffer_copy, buffer);
			strcpy(entry->key, strsep(&buffer_copy, "="));
			strncpy(entry->value, buffer_copy, strlen(buffer_copy));
			entry->value[strlen(buffer_copy)] = '\0';

			// check if entries are correct if it is a entry
			if(entry->key != NULL)
			{
				is_valid = 1;
			}
		}
	
		free(buffer_copy_pointer);
	}
	else
	{
		perror("couldn't allocate memory\n");
	}

	return is_valid;
}

int is_empty_line(char line[MAX_BUFFER_SIZE])
{
	int i, is_empty = 1, eol = 0;

	for(i = 0; i < MAX_BUFFER_SIZE && is_empty && ! eol; i++)
	{
		if(line[i] == '\n' || line[i] == '\0')
		{
			eol = 1;
		}
		else
		{
			// is empty if it is space, tab, \n or \0
			is_empty = (line[i] == ' ' || line[i] == '\t');
		}
	}

	return is_empty;
}

// line begins with a `#'
int is_comment_line(char line[MAX_BUFFER_SIZE])
{
	int is_comment = 0;
	int error = 0;
	int eol = 0;
	int i;

	for(i = 0; i < MAX_BUFFER_SIZE && ! is_comment && ! error && ! eol; i++)
	{
		if(line[i] == '#')
		{
			is_comment = 1;
		}
		else if(line[i] == '\n' || line[i] == '\0')
		{
			// EOL reached
			eol = 1;
		}
		else if(line[i] != ' ')
		{
			error = 1;
		}
	}

	return is_comment;
}

//
int sync_entries(config_entry entries[MAX_ENTRIES], servinfo *conf, int num_entries)
{
	int path_ok = 0,
	    mode_ok = 0,
	    port_ok = 0,
		i;

	for(i = 0; i < num_entries && ! is_entry_empty(entries[i]); i++)
	{
		if(! strcmp(HTDIR_KEY, entries[i].key))
		{
			path_ok = 1;
			strcpy(conf->htdir, entries[i].value);
		}
		else if(! strcmp(PORT_KEY, entries[i].key))
		{
			conf->port = atoi(entries[i].value);

			// the port must be a valid int (atoi() > 0)
			if(conf->port > 0)
			{
				port_ok = 1;
			}
		}
        else if(! strcmp(MODE_KEY, entries[i].key))
        {
            mode_ok = 1;
			
			// check what values
	        if(! strcmp(MODE_THREAD_VALUE, entries[i].value))
			{
				conf->mode = MODE_THREAD;
			}
			else if(! strcmp(MODE_FORK_VALUE, entries[i].value))
			{
				conf->mode = MODE_FORK;
			}
			else if(! strcmp(MODE_NONE_VALUE, entries[i].value))
			{
				conf->mode = MODE_NONE;
			}
			else
			{
				mode_ok = 0;
			}
        }
	}

	return path_ok && mode_ok && port_ok;
}

int is_entry_empty(config_entry e)
{
	return e.key == NULL || e.value == NULL;
}

void error_case(char description[MAX_ERROR_LEN], int is_error)
{
	if(is_error)
	{
		fprintf(stderr, "%s", description);
	}
}

#ifdef DEBUG_CONFIG

// test out the config file
int main()
{
	servinfo conf;
	int err=process_config("/Users/Marcos/Dev/GITT/SSOO/http-server/conf", &conf);

	printf("e: %d | config.port: %d | config.htdir: %s | config.mode: %d\n", err, conf.port, conf.htdir, conf.mode);

	return 0;
}

#endif
