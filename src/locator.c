#include "locator.h"

// testcollection (collection)
// testcollection.subcollection (collection)
// testcollection.subcollection.key (key)

// db: collection.subcollection.subsubcollection.entry?
store_entry *locate_entry(char key[MAX_KEY_SIZE], char *dbname, store_db *db)
{
	char collection[MAX_KEY_SIZE];
	extract_collection(collection, key);
	store_collection *col = locate_collection(collection, db);

	while(col->ent != NULL && 0 != strcmp(key, col->ent->key))
	{
		col->ent = col->ent->next;
	}

	return col->ent;
}

store_collection *locate_collection(char path[MAX_KEY_SIZE], store_db *db)
{
	char parent[MAX_KEY_SIZE];
	char name[MAX_KEY_SIZE];
	store_collection *collection = NULL;

	// get the name of our current collection name
	extract_name(name, path);

	if(0 == extract_collection(parent, path))
	{
		// find the root
		while(db->col != NULL && 0 != strcmp(name, db->col->name))
		{
			// the first db->col is the root collection of everyone (name: ""),
			// so search among its brothers if we're not refering to it
			db->col = db->col->brother;
		}

		// our root collection (NULL if not found)
		collection = db->col;
	}
	else
	{
		// our parent
		collection = locate_collection(parent, db);

		// did we even find the parent of this collection?
		if(collection != NULL)
		{
			// now find it's childrens
			while(collection->children != NULL
			      && 0 != strcmp(name, collection->children->name))
			{
				collection->children = collection->children->brother;
			}
		}
	}

	return collection;
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
