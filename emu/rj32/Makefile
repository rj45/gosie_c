CFLAGS ?= -std=c11 -Wall -Werror
CC ?= clang

SRCS = emurj.c inst.c bus.c cpu.c

.PHONY: all clean run run

all: test emurj

test: $(SRCS) test.c
	$(CC) $(CFLAGS) -o test $^
	./test
	rm test

emurj: $(SRCS) main.c
	$(CC) $(CFLAGS) -o emurj $^

run: emurj
	./emurj

clean:
	rm -f test
