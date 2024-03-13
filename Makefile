CFLAGS ?= -std=c11 -Wall -Werror
CC ?= clang

.PHONY: all clean run

all: gosie

gosie: gosie.c
	$(CC) $(CFLAGS) -o gosie gosie.c

run: gosie
	./gosie

clean:
	rm -f gosie tmp.asm tmp.rom

