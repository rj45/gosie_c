CFLAGS ?= -std=c11 -Wall -Werror
CC ?= clang

SRCS = ast.c token.c parser.c ir.c compile.c stb_ds.c emu/rj32/emurj.c emu/rj32/inst.c emu/rj32/bus.c emu/rj32/cpu.c

.PHONY: all clean run

all: gosie

gosie: $(SRCS) gosie.c gosie.h
	$(CC) $(CFLAGS) -o gosie $(SRCS) -Llibcustomasm/target/release/ -llibcustomasm gosie.c

test: test.c $(SRCS) gosie_test.c utest.h gosie.h
	$(CC) $(CFLAGS) -o test $(SRCS) -Llibcustomasm/target/release/ -llibcustomasm test.c gosie_test.c

run: gosie
	./gosie

clean:
	rm -f gosie tmp.asm tmp.rom

