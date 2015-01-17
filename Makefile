SRCDIR = ./src
BINDIR = ./bin

all: create_bin keystore_clear_memory keystore keystored keystore_elapsed
	cp $(SRCDIR)/daemon.sh $(BINDIR)/keystored
	cp $(SRCDIR)/keystore.sh $(BINDIR)/keystore

keystored: $(SRCDIR)/daemon.c $(SRCDIR)/daemon.h $(SRCDIR)/server.c $(SRCDIR)/server.h $(SRCDIR)/memory.c $(SRCDIR)/memory.h $(SRCDIR)/common.c $(SRCDIR)/common.h $(SRCDIR)/database.c $(SRCDIR)/database.h $(SRCDIR)/types.h $(SRCDIR)/sems.c $(SRCDIR)/sems.h
	gcc -W -Wall $(SRCDIR)/daemon.c $(SRCDIR)/server.c $(SRCDIR)/memory.c $(SRCDIR)/common.c $(SRCDIR)/database.c $(SRCDIR)/sems.c -o $(BINDIR)/keystored.bin -lpthread

keystore: $(SRCDIR)/keystore.c $(SRCDIR)/keystore.h $(SRCDIR)/client.c $(SRCDIR)/client.h $(SRCDIR)/common.c $(SRCDIR)/common.h $(SRCDIR)/sems.c $(SRCDIR)/sems.h
	gcc -W -Wall $(SRCDIR)/keystore.c $(SRCDIR)/client.c $(SRCDIR)/memory.c $(SRCDIR)/common.c $(SRCDIR)/database.c $(SRCDIR)/sems.c -o $(BINDIR)/keystore.bin -lpthread

keystore_elapsed: $(SRCDIR)/elapsed.c
	gcc -W -Wall $(SRCDIR)/elapsed.c -o $(BINDIR)/keystore_elapsed.bin

keystore_clear_memory: $(SRCDIR)/clear.c $(SRCDIR)/common.c $(SRCDIR)/common.h $(SRCDIR)/types.h $(SRCDIR)/sems.c $(SRCDIR)/sems.h
	gcc -W -Wall $(SRCDIR)/clear.c $(SRCDIR)/common.c $(SRCDIR)/sems.c -o $(BINDIR)/keystore_clear_memory.bin -lpthread

create_bin:
	mkdir -p $(BINDIR)

clear:
	$(BINDIR)/clear_memory.bin
	rm $(BINDIR)/*

install:
	cp $(BINDIR)/* /usr/bin
