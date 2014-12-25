#ifndef __KEYSTORE_DATABASE__

store_db *create_db(char name[MAX_DB_SIZE], store_db **dbs);
store_entry *create_entry(char key[MAX_KEY_SIZE], store_db **db);
store_entry *locate_entry(char path[MAX_KEY_SIZE], store_db *db);
store_db *locate_db(char *name, store_db *db);

#endif
