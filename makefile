CURDIR = $(shell pwd)
SRCDIR = .
OUTFILE = pixql
CC = gcc
DEBUGGER = gdb -q
CFLAGS = -Wall -std=c99
LFLAGS = -Wall -lm -std=c99
DEBUGFLAGS = -ggdb3 -O0
PRODFLAGS = -O3
ALL_HEADERS:=$(wildcard $(CURDIR)/$(SRCDIR)/*.h)
ALL_SOURCES:=$(wildcard $(CURDIR)/$(SRCDIR)/*.c)
HEADERS:=$(ALL_HEADERS)
SOURCES:=$(filter-out %/test.c, $(ALL_SOURCES))

INPUT_FILE = ~/Desktop/input.bmp
INPUT_FILE = ~/Desktop/simple_input.bmp
OUTPUT_FILE = ~/Desktop/output.bmp

SAMPLE_QUERY = "WHITE; SELECT WHERE COL%100 < 50; OPERATE SET R = R; OPERATE SET G = G; OPERATE SET B = B; SELECT WHERE OUT.R = 255 AND OUT.G = 255 AND OUT.B = 255; OPERATE SET G = 255-G;"
SAMPLE_QUERY = "SELECT WHERE COL > 2; OPERATE SET G = 255;"
SAMPLE_QUERY = "COPY; SELECT WHERE COL > 1; OPERATE SET B = 255;"
SAMPLE_QUERY = "WHITE(256,256); OPERATE SET G = IN[INT(COL/2),INT(ROW/2)].G;"
SAMPLE_QUERY = "OPERATE SET G = G;"

SAMPLE_ARGS = -i ~/Desktop/cage.bmp -o ~/Desktop/new.bmp -q "OPERATE SET G = G;"
SAMPLE_ARGS = -i ~/Desktop/cage.bmp -o ~/Desktop/cage_out.bmp -q "COPY(100,20); SELECT WHERE ROW < 5; OPERATE SET G = 255;"
SAMPLE_ARGS = -i ~/Desktop/cage_imagick.bmp -o ~/Desktop/cage_out.bmp -q "SELECT WHERE ROW%2; OPERATE SET R = 255;"
SAMPLE_ARGS = -i ~/Desktop/cage.bmp -o ~/Desktop/cage_out.bmp "WHITE(WIDTH/4,HEIGHT/4); OPERATE SET COLOR = IN[COL*4,ROW*4].COLOR;"
SAMPLE_ARGS = -i ../color_test.bmp -o ../pixql2.bmp $(SAMPLE_QUERY)

$(OUTFILE): $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(LFLAGS) $(SOURCES) -o $(OUTFILE)

run: $(OUTFILE)
	./$(OUTFILE)

runargs: $(OUTFILE)
	./$(OUTFILE) -i $(INPUT_FILE) -o $(OUTPUT_FILE) -q $(SAMPLE_QUERY)

runsetargs: $(OUTFILE)
	./$(OUTFILE) $(SAMPLE_ARGS)

$(OUTFILE).dSYM: $(HEADERS) $(SOURCES)
	$(CC) $(DEBUGFLAGS) $(CFLAGS) $(LFLAGS) $(SOURCES) -o $(OUTFILE)

debug: $(OUTFILE).dSYM
	$(DEBUGGER) $(OUTFILE)

debugsetargs: $(OUTFILE).dSYM
	$(DEBUGGER) --args ./$(OUTFILE) $(SAMPLE_ARGS)

debugargs: $(OUTFILE).dSYM
	$(DEBUGGER) --args ./$(OUTFILE) -i $(INPUT_FILE) -o $(OUTPUT_FILE) -q $(SAMPLE_QUERY)

debugargsfile: $(OUTFILE).dSYM
	$(DEBUGGER) --args ./$(OUTFILE) -i $(INPUT_FILE) -o $(OUTPUT_FILE) -qf info.txt

TEST_OUTFILE=pixql_test
TEST_HEADERS:=$(ALL_HEADERS)
TEST_SOURCES:=$(filter-out %/main.c, $(ALL_SOURCES))

$(TEST_OUTFILE): $(TEST_HEADERS) $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(LFLAGS) $(TEST_SOURCES) -o $(TEST_OUTFILE)

test: $(TEST_OUTFILE)
	./$(TEST_OUTFILE)

$(TEST_OUTFILE).dSYM: $(TEST_HEADERS) $(TEST_SOURCES)
	$(CC) $(DEBUGFLAGS) $(CFLAGS) $(LFLAGS) $(TEST_SOURCES) -o $(TEST_OUTFILE)

testdebug: $(TEST_OUTFILE).dSYM
	$(DEBUGGER) $(TEST_OUTFILE)

clean:
	for f in $(OUTFILE) $(OUTFILE).dSYM $(TEST_OUTFILE) $(TEST_OUTFILE).dSYM ; do if [ -d $$f ] ; then rm -r $$f ; elif [ -f $$f ] ; then rm $$f ; fi ; done

testabunch:
	for i in ../bmps/*; do ./pixql -i $$i -o `echo $$i | sed s/bmps/bmps_out/` -q "OPERATE SET G = G;"; done;

