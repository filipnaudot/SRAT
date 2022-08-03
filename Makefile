
CC = gcc

OBJDIR = obj
SRCDIR = src
OUTPUTDIR = out

OUTPUTNAME_REMOTE = remote
OUTPUTNAME_LOCAL = local


all: local remote



local: $(SRCDIR)/local.o $(SRCDIR)/transfer.o
	mkdir -p $(OUTPUTDIR)
	$(CC) -o $(OUTPUTNAME_LOCAL) $(SRCDIR)/local.o $(SRCDIR)/transfer.o
	mv $(OUTPUTNAME_LOCAL) $(OUTPUTDIR)
	mkdir -p $(OBJDIR)
	mv $(SRCDIR)/*.o $(OBJDIR)

remote: $(SRCDIR)/remote.o $(SRCDIR)/transfer.o
	mkdir -p $(OUTPUTDIR)
	$(CC) -o $(OUTPUTNAME_REMOTE) $(SRCDIR)/remote.o $(SRCDIR)/transfer.o
	mv $(OUTPUTNAME_REMOTE)$(OUTPUTDIR)
	mkdir -p $(OBJDIR)
	mv $(SRCDIR)/*.o $(OBJDIR)



local.o: $(SRCDIR)/local.c
	$(CC) -c $(SRCDIR)/local.c

remote.o: $(SRCDIR)/remote.c
	$(CC) -c $(SRCDIR)/remote.c

transfer.o: $(SRCDIR)/transfer.c
	$(CC) -c $(SRCDIR)/transfer.c



clean:
	rm -rf $(OUTPUTDIR)
	rm -rf $(OBJDIR)
