CC = gcc
CFLAGS = -Wall -gstabs -lpanel -lncurses -pedantic -std=c99

vex: vex.c lib/view.c lib/route.c lib/buffer.c
	$(CC) $^ $(CFLAGS) -o $@

install:
	mkdir -p /usr/share/local/vex
	mv vex /usr/share/local/vex/vex

clean:
	rm vex
