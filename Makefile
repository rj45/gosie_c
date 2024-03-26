CFLAGS ?= -std=c11 -Wall -Werror
CC ?= clang
LIBS = -Llibcustomasm/target/aarch64-apple-darwin/debug -llibcustomasm

SRCS = src/ast.c src/token.c src/parser.c src/ir.c src/compile.c \
	src/stb_ds.c emu/rj32/emurj.c emu/rj32/inst.c emu/rj32/bus.c \
	emu/rj32/cpu.c

.PHONY: all clean run

all: gosie

gosie: $(SRCS) src/gosie.c src/gosie.h
	$(CC) $(CFLAGS) -o gosie $(SRCS) $(LIBS) src/gosie.c

test: $(SRCS) src/test.c src/gosie_test.c src/utest.h src/gosie.h
	$(CC) $(CFLAGS) -o test $(SRCS) $(LIBS) src/test.c src/gosie_test.c

run: gosie
	./gosie

clean:
	rm -rf gosie test tmp.asm tmp.rom *.dSYM

