CURDIR = ~/Desktop/pixQL
SRCDIR = .
OUTFILE = pixql
CC = gcc
DEBUG = -ggdb3
DEBUGGER = gdb -q
CFLAGS = -Wall
LFLAGS = -Wall
ALL_HEADERS:=$(wildcard $(CURDIR)/$(SRCDIR)/*.h)
ALL_SOURCES:=$(wildcard $(CURDIR)/$(SRCDIR)/*.c)
HEADERS:=$(ALL_HEADERS)
SOURCES:=$(filter-out %/test.c, $(ALL_SOURCES))

$(OUTFILE): $(HEADERS) $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) $(LFLAGS) -o $(OUTFILE)

run: $(OUTFILE)
	./$(OUTFILE)

$(OUTFILE).dSYM: $(HEADERS) $(SOURCES)
	$(CC) $(DEBUG) $(SOURCES) $(CFLAGS) $(LFLAGS) -o $(OUTFILE)

debug: $(OUTFILE).dSYM
	$(DEBUGGER) $(OUTFILE)


TEST_OUTFILE=pixql_test
TEST_HEADERS:=$(ALL_HEADERS)
TEST_SOURCES:=$(filter-out %/main.c, $(ALL_SOURCES))

$(TEST_OUTFILE): $(TEST_HEADERS) $(TEST_SOURCES)
	$(CC) $(TEST_SOURCES) $(CFLAGS) $(LFLAGS) -o $(TEST_OUTFILE)

test: $(TEST_OUTFILE)
	./$(TEST_OUTFILE)

$(TEST_OUTFILE).dSYM: $(TEST_HEADERS) $(TEST_SOURCES)
	$(CC) $(DEBUG) $(TEST_SOURCES) $(CFLAGS) $(LFLAGS) -o $(TEST_OUTFILE)

test_debug: $(TEST_OUTFILE).dSYM
	$(DEBUGGER) $(TEST_OUTFILE)

clean:
	for f in $(OUTFILE) $(OUTFILE) $(TEST_OUTFILE) $(TEST_OUTFILE) ; do if [ -d $$f ] ; then rm -r $$f ; elif [ -f $$f ] ; then rm $$f ; fi ; done

