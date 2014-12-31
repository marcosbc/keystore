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
	*iterator = (store_db *) calloc(1, sizeof(store_db));

	if(*iterator != NULL)
	{
		strcpy((*iterator)->name, name);
		(*iterator)->ent = NULL;
		(*iterator)->next = NULL;
		
		DEBUG_PRINT("created db %s at %p, prev is %p\n", name, *iterator, prev);

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
	#ifdef __DEBUG__
	DEBUG_PRINT("store tree BEFORE");
	print_store_tree(db);
	#endif

	store_entry *entry = NULL;
	store_entry **iterator = &((*db)->ent);
	store_entry **complete_iterator = &((*db)->ent); // iterate all entry elements

	while(*complete_iterator != NULL)
	{
		complete_iterator = &((*complete_iterator)->next);
	}

	// find the last element entry
	while(*iterator != NULL)
	{
		iterator = &((*iterator)->brother);
	}

	entry = (store_entry *) calloc(1, sizeof(store_entry));
	DEBUG_PRINT("alloc: %p\n", entry);
	
	if(entry != NULL)
	{
		DEBUG_PRINT("setting value for entry\n");
		// add basic fields
		*iterator = entry;
		*complete_iterator = entry;
		strcpy(entry->key, key);
		entry->val = NULL;
		entry->next = NULL;
		entry->brother = NULL;
		DEBUG_PRINT("values set for entry\n");
	}

	#ifdef __DEBUG__
	DEBUG_PRINT("store tree AFTER");
	print_store_tree(db);
	#endif

	return entry;
}

// locate an entry in the database
store_entry *locate_entry(char key[MAX_KEY_SIZE], store_db *db)
{
	// find our collection
	while(db->ent != NULL && 0 != strcmp(key, db->ent->key))
	{
		db->ent = db->ent->brother;
	}

	return db->ent;
}

// db?
store_db *locate_db(char *name, store_db *db)
{
	while(db != NULL && 0 != strncmp(name, db->name, MAX_DB_SIZE))
	{
		db = db->next;
	}

	return db;
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
			p = p->next;
		}

		iterator = &((*iterator)->next);
	}
}
#endif
