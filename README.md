==== DESCRIPTION ====

We have two different modes of usage:
- set: Creates the specified databases if non-existant, and stores the specified values inside them
- get: Searches the key in the databases, and returns the values found in each

==== TECHNIQUES USED ====

fork:
- parent: alter the database file
- children: alter the database in memory

threads: for searching through multiple databases

semaphores: for altering memory operations

==== USAGE ====

    ??? $ keystore start
	??? $ keystore stop

    $ keystore set KEY VALUE DB1[ DB2[ ...]]
    e.g.:
	$ keystore set my_key my_value db1 db2 db3 db4 db5

    $ keystore get KEY DB1[ DB2[ ...]]
	e.g.:
	$ keystore get my_key db1 db2 db3 db4 db5
