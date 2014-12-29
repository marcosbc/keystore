#include <strings.h>
#include "common.h"
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
		
		// add basic fields
		if(prev != NULL)
		{
			prev->next = *iterator;
		}
	}

	return *iterator;
}

store_entry *create_entry(char key[MAX_KEY_SIZE], store_db **db)
{
	store_entry *entry = NULL;
	store_entry **iterator = &((*db)->ent);

	// find the last element entry
	while(*iterator != NULL)
	{
		iterator = &((*iterator)->next);
	}

	entry = (store_entry *) malloc(sizeof(store_entry));
		DEBUG_PRINT("alloc: %p\n", entry);
	if(entry != NULL)
	{
		DEBUG_PRINT("setting value for entry\n");
		// add basic fields
		*iterator = entry;
		strcpy(entry->key, key);
		entry->val = NULL;
		entry->next = NULL;
		DEBUG_PRINT("values set for entry\n");
	}

	return entry;
}

// db: collection.subcollection.subsubcollection.entry?
store_entry *locate_entry(char key[MAX_KEY_SIZE], store_db *db)
{
	// find our collection
	while(db->ent != NULL && 0 != strcmp(key, db->ent->key))
	{
		db->ent = db->ent->next;
	}

	return db->ent;
}

// db?
store_db *locate_db(char *name, store_db *db)
{
	while(db != NULL && 0 != strncmp(name, db->name,
	                                 (size_t) min((int) strlen(name),
									              MAX_DB_SIZE)))
	{
		db = db->next;
	}

	return db;
}

void free_tree(store_db **dbs)
{
	store_db **prev_db = NULL;
	store_entry **entry = NULL;
	store_entry **prev = NULL;

	// see what our dbs contains now
	while(*dbs != NULL)
	{
		entry = &((*dbs)->ent);
		while(*entry != NULL)
		{
			// free it's value
			free((*entry)->val);
			prev = entry;
			entry = &((*entry)->next);
			
			// go to the next one
			free(*prev);
			*prev = NULL;
		}

		prev_db = dbs;
		dbs = &((*dbs)->next);
		free(*prev_db);
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

void print_store_tree(store_db *dbs)
{
	store_entry *p = NULL;

	// see what our dbs contains now
	while(dbs != NULL)
	{
		printf("\n\n--- database %s ---\n", dbs->name);
		p = dbs->ent;
		while(p != NULL)
		{
			printf("* %s=%s\n", p->key, p->val);
			p = p->next;
		}

		dbs = dbs->next;
	}
}
#endif
