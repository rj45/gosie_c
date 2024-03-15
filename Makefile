CFLAGS ?= -std=c11 -Wall -Werror
CC ?= clang

SRCS = ast.c token.c

.PHONY: all clean run

all: gosie

gosie: $(SRCS) gosie.c
	$(CC) $(CFLAGS) -o gosie $(SRCS) gosie.c

test: test.c $(SRCS) gosie_test.c utest.h
	$(CC) $(CFLAGS) -o test $(SRCS) test.c gosie_test.c

run: gosie
	./gosie

clean:
	rm -f gosie tmp.asm tmp.rom

