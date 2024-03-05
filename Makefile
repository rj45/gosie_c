CFLAGS = -std=c11 -Wall -Werror

.PHONY: all clean run runemu

all: test emu

test: test.c
	gcc $(CFLAGS) -o test test.c

run: test
	./test

emu: emu.c
	gcc $(CFLAGS) -o emu emu.c

runemu: emu
	./emu

clean:
	rm -f test

