CC = gcc

OBJDIR = obj
SRCDIR = src
OUTPUTDIR = out

OUTPUTNAME_REMOTE = remote
OUTPUTNAME_LOCAL = local


all: local remote



local: local.o transfer.o
	mkdir -p $(OUTPUTDIR)
	$(CC) -o $(OUTPUTNAME_LOCAL) $(OBJDIR)/local.o $(OBJDIR)/transfer.o
	mv $(OUTPUTNAME_LOCAL) $(OUTPUTDIR)

remote: remote.o transfer.o
	mkdir -p $(OUTPUTDIR)
	$(CC) -o $(OUTPUTNAME_REMOTE) $(OBJDIR)/remote.o $(OBJDIR)/transfer.o
	mv $(OUTPUTNAME_REMOTE) $(OUTPUTDIR)



local.o: $(SRCDIR)/local.c
	$(CC) -c $(SRCDIR)/local.c
	mkdir -p $(OBJDIR)
	mv local.o $(OBJDIR)

remote.o: $(SRCDIR)/remote.c
	$(CC) -c $(SRCDIR)/remote.c
	mkdir -p $(OBJDIR)
	mv remote.o $(OBJDIR)

transfer.o: $(SRCDIR)/transfer.c
	$(CC) -c $(SRCDIR)/transfer.c
	mkdir -p $(OBJDIR)
	mv transfer.o $(OBJDIR)



clean:
	rm -rf $(OUTPUTDIR)
	rm -rf $(OBJDIR)
