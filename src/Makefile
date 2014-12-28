all: keystore keystored elapsed

keystored: daemon.* server.* memory.* disk.* common.* database.*
	gcc -W -Wall daemon.c server.c memory.c disk.c common.c database.c -o keystored -lpthread

keystore: keystore.* client.* memory.* disk.* common.* database.*
	gcc -W -Wall keystore.c client.c memory.c disk.c common.c database.c -o keystore -lpthread

elapsed:
	gcc -W -Wall elapsed.c -o elapsed
