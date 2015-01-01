SRCDIR = ./src
BINDIR = ./bin

all: clear_memory clear keystore keystored elapsed

keystored: $(SRCDIR)/daemon.* $(SRCDIR)/server.* $(SRCDIR)/memory.* $(SRCDIR)/disk.* $(SRCDIR)/common.* $(SRCDIR)/database.*
	gcc -W -Wall $(SRCDIR)/daemon.c $(SRCDIR)/server.c $(SRCDIR)/memory.c $(SRCDIR)/disk.c $(SRCDIR)/common.c $(SRCDIR)/database.c -o $(BINDIR)/keystored -lpthread

keystore: $(SRCDIR)/keystore.* $(SRCDIR)/client.* $(SRCDIR)/memory.* $(SRCDIR)/disk.* $(SRCDIR)/common.* $(SRCDIR)/database.*
	gcc -W -Wall $(SRCDIR)/keystore.c $(SRCDIR)/client.c $(SRCDIR)/memory.c $(SRCDIR)/disk.c $(SRCDIR)/common.c $(SRCDIR)/database.c -o $(BINDIR)/keystore -lpthread

elapsed: $(SRCDIR)/elapsed.c
	gcc -W -Wall $(SRCDIR)/elapsed.c -o $(BINDIR)/elapsed

clear_memory: $(SRCDIR)/clear.c $(SRCDIR)/common.c $(SRCDIR)/common.h
	gcc -W -Wall $(SRCDIR)/clear.c $(SRCDIR)/common.c -o $(BINDIR)/clear_memory -lpthread

clear:
	./bin/clear_memory
	rm ./bin/clear_memory ./bin/keystored ./bin/keystore ./bin/elapsed
