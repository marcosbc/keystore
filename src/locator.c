#include "locator.h"

// db: collection.subcollection.subsubcollection.entry?
store_entry *locate_entry(char *key, store_db *db)
{
	store_collection *col = locate_collection();

	while(col->ent != NULL)
	{
		if(0 != strcmp(key, col->ent->key))
		{
			col->ent = col->ent->next;
		}
	}

	return col->ent;
}

// db: collection.subcollection.subsubcollection?
store_collection *locate_collection(char *name, store_db *db)
{
	while(db->col != NULL)
	{
		if(0 != strcmp(name, db->col->name))
		{
			db->col = db->col->next;
		}
	}

	return db->col;
}

// db?
store_db *locate_db(char *name, store_db *db)
{
	while(db != NULL)
	{
		if(0 != strncmp(name, db, (size_t) min((int) strlen(name), MAX_DB_SIZE)))
		{
			db = db->next;
		}
	}

	return db;
}
