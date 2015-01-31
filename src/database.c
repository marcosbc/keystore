#include <strings.h>
#include "common.h"
#include "types.h"
#include "database.h"

store_db *create_db(char name[MAX_DB_SIZE], store_db **dbs)
{
	store_db *prev = NULL;
	store_db **iterator = dbs;

	// find the last element
	while(*iterator != NULL)
	{
		prev = *iterator;
		iterator = &((*iterator)->next);
	}

	// now create it
	*iterator = (store_db *) malloc(sizeof(store_db));

	if(*iterator != NULL)
	{
		strcpy((*iterator)->name, name);
		(*iterator)->ent = NULL;
		(*iterator)->next = NULL;
		
		DEBUG_PRINT("created db %s: %p, prev: %p\n", name, *iterator, prev);

		// add basic fields
		if(prev != NULL)
		{
			prev->next = *iterator;
		}
	}

	return *iterator;
}

store_db *locate_db(char *name, store_db *db)
{
	store_db *iterator = db;

	while(iterator != NULL && 0 != strncmp(name, iterator->name, MAX_DB_SIZE))
	{
		iterator = iterator->next;
	}

	return iterator;
}

store_entry *create_entry(char key[MAX_KEY_SIZE], store_db *db)
{
	store_entry *entry = NULL;
	store_entry **iterator = &(db->ent); // only specific db entries
	store_entry **complete_iterator = &(db->ent); // all entries

	while(*complete_iterator != NULL)
	{
		complete_iterator = &((*complete_iterator)->next);
	}

	// find the last element entry
	while(*iterator != NULL)
	{
		iterator = &((*iterator)->brother);
	}

	entry = (store_entry *) malloc(sizeof(store_entry));
	
	if(entry != NULL)
	{
		DEBUG_PRINT("setting value for entry %p\n", entry);

		// add basic fields
		*iterator = entry;
		*complete_iterator = entry;
		strcpy(entry->key, key);
		entry->val = NULL;
		entry->next = NULL;
		entry->brother = NULL;
	}
	
	return entry;
}

// locate an entry in the database
store_entry *locate_entry(char key[MAX_KEY_SIZE], store_db *db)
{
	DEBUG_PRINT("notice: locating entry\n");
	store_entry *iterator = db->ent;

	// find our collection
	while(iterator != NULL && 0 != strcmp(key, iterator->key))
	{
		iterator = iterator->brother;
	}

	return iterator;
}

// delete an entry in the database, don't give error if it fails
void delete_entry(char key[MAX_KEY_SIZE], store_db *db)
{
	DEBUG_PRINT("notice: deleting entry\n");
	store_entry **iterator = &(db->ent);
	store_entry *aux = NULL;

	// find our collection
	while(*iterator != NULL && 0 != strcmp(key, (*iterator)->key))
	{
		iterator = &((*iterator)->brother);
	}

	if(*iterator != NULL)
	{
		DEBUG_PRINT("notice: %s: entry found for deletion\n", db->name);

		// set auxiliary ptr, and link previous to next element
		aux = *iterator;
		*iterator = (*iterator)->brother;

		// free the value and the iterator itself
		free(aux->val);
		free(aux);
	}
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
    printf("------------------\n\n");

}

void print_store_tree(store_db **dbs)
{
	store_entry *p = NULL;
	store_db **iterator = dbs;

	// see what our dbs contains now
	while(*iterator != NULL)
	{
		printf("\n\n--- database %s ---\n", (*iterator)->name);
		p = (*iterator)->ent;
		while(p != NULL)
		{
			printf("* %s=%s\n", p->key, p->val);
			p = p->brother;
		}

		iterator = &((*iterator)->next);
	}
}
#endif
