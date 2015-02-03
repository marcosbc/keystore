SRCDIR = ./src
BINDIR = ./bin

all: create_bin keystore keystored
	cp $(SRCDIR)/daemon.sh $(BINDIR)/keystored
	cp $(SRCDIR)/keystore.sh $(BINDIR)/keystore

keystored: $(SRCDIR)/daemon.c $(SRCDIR)/daemon.h $(SRCDIR)/server.c $(SRCDIR)/server.h $(SRCDIR)/memory.c $(SRCDIR)/memory.h $(SRCDIR)/common.c $(SRCDIR)/common.h $(SRCDIR)/database.c $(SRCDIR)/database.h $(SRCDIR)/types.h $(SRCDIR)/sems.c $(SRCDIR)/sems.h
	gcc -W -Wall $(SRCDIR)/daemon.c $(SRCDIR)/server.c $(SRCDIR)/memory.c $(SRCDIR)/common.c $(SRCDIR)/database.c $(SRCDIR)/sems.c -o $(BINDIR)/keystored.bin -pthread

keystore: $(SRCDIR)/keystore.c $(SRCDIR)/keystore.h $(SRCDIR)/client.c $(SRCDIR)/client.h $(SRCDIR)/common.c $(SRCDIR)/common.h
	gcc -W -Wall $(SRCDIR)/keystore.c $(SRCDIR)/client.c $(SRCDIR)/common.c -o $(BINDIR)/keystore.bin -pthread

create_bin:
	mkdir -p $(BINDIR)

clear:
	$(BINDIR)/clear_memory.bin
	rm $(BINDIR)/*

install:
	cp $(BINDIR)/* /usr/bin
