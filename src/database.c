#include <strings.h>
#include "common.h"
#include "database.h"

store_db *create_db(char name[MAX_DB_SIZE], store_db **dbs)
{
	store_db **iterator = dbs;
	store_db *db = NULL;

	// find the last element
	while((*iterator)->next != NULL)
	{
		iterator = &((*dbs)->next);
	}

	// now create it
	db = (store_db *) store_data(sizeof(store_db));

	if(db != NULL)
	{
		// add basic fields
		(*iterator)->next = db;
		strcpy(db->name, name);
		db->ent = NULL;
		db->next = NULL;
	}

	return db;
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

	entry = (store_entry *) store_data(sizeof(store_entry));
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
	while(db != NULL)
	{
		if(0 != strncmp(name, db->name, (size_t) min((int) strlen(name),
		                                             MAX_DB_SIZE)))
		{
			db = db->next;
		}
	}

	return db;
}
