CFLAGS=-Wall -pedantic -std=c99
DEBUG=-g
INCLUDES=-I./include
INTERCEPT=-DINTERCEPT
GUI=-DGUI
# postscript backend
PS=-DPOSTSCRIPT 
LIBS=`pkg-config --cflags --libs gtk+-2.0`

.PHONY: all clean tests

all: parse interp extension tests

parse: src/parse.c src/parser.c
	gcc $(CFLAGS) $(DEBUG) $(INCLUDES) -o parse src/parse.c src/parser.c

interp: src/interp.c src/interpreter.c src/postscript.c
	gcc $(CFLAGS) $(DEBUG) $(INCLUDES) -o interp src/interp.c src/interpreter.c src/postscript.c $(PS)

extension: src/extension.c src/interpreter.c
	gcc $(CFLAGS) $(DEBUG) $(INCLUDES) $(LIBS) -o extension \
		src/extension.c src/interpreter.c  src/overrides.c $(GUI)  -lm

test_parser: tests/test_parser.c src/parser.c
	gcc $(CFLAGS) $(DEBUG) $(INCLUDES) -o test_parser \
		tests/test_parser.c src/parser.c

test_interpreter: tests/test_interpreter.c src/interpreter.c
	gcc $(CFLAGS) $(DEBUG) $(INCLUDES) -o test_interpreter \
		tests/test_interpreter.c src/interpreter.c src/postscript.c $(PS)

test_psr_malloc: tests/test_psr_malloc.c src/parser.c src/overrides.c
	gcc $(CFLAGS) $(DEBUG) $(INCLUDES) -o test_psr_malloc \
		tests/test_psr_malloc.c src/parser.c src/overrides.c $(INTERCEPT)

test_int_malloc: tests/test_int_malloc.c src/interpreter.c src/overrides.c
	gcc $(CFLAGS) $(DEBUG) $(INCLUDES) -o test_int_malloc \
		tests/test_int_malloc.c src/interpreter.c src/overrides.c src/postscript.c $(INTERCEPT) $(PS)

tests: test_parser test_interpreter	test_psr_malloc test_int_malloc

clean:
	rm -rf test_* parse interp extension
	rm -rf *.dSYM # for mac os
