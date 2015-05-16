CURDIR = ~/Desktop/pixQL
SRCDIR = .
OUTFILE = pixql
CC = gcc
DEBUG = -g
DEBUGGER = gdb -q
CFLAGS = -Wall
LFLAGS = -Wall
HEADERS=$(wildcard $(CURDIR)/$(SRCDIR)/*.h)
SOURCES=$(wildcard $(CURDIR)/$(SRCDIR)/*.c)

test: $(OUTFILE)
	./$(OUTFILE) -i ~/Desktop/test.bmp -o ~/Desktop/out_test.bmp "test query"

$(OUTFILE): $(HEADERS) $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) $(LFLAGS) -o $(OUTFILE)

run: $(OUTFILE)
	./$(OUTFILE)

$(OUTFILE).dSYM: $(HEADERS) $(SOURCES)
	$(CC) $(DEBUG) $(SOURCES) $(CFLAGS) $(LFLAGS) -o $(OUTFILE)

debug: $(OUTFILE).dSYM
	$(DEBUGGER) $(OUTFILE)

