CC=gcc
CFLAGS=-g -I. `pkg-config --cflags glib-2.0`

lisp: lisp.o env.o symbol.o mutable_hash.o
	$(CC) -o lisp lisp.o env.o symbol.o mutable_hash.o -I. -lgc -lreadline `pkg-config --libs glib-2.0`

clean:
	rm -f *.o

# vim: set tabstop=4 shiftwidth=4 noexpandtab:
