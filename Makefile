CC=gcc
CFLAGS=-g -I. `pkg-config --cflags gio-2.0`

lisp: lisp.o env.o symbol.o keyword.o mutable_hash.o vector.o hashmap.o persistenthashmap.o numbers.o
	$(CC) -o lisp lisp.o env.o symbol.o keyword.o mutable_hash.o vector.o hashmap.o persistenthashmap.o numbers.o -I. -lgc -lreadline `pkg-config --libs gio-2.0`

clean:
	rm -f *.o

# vim: set tabstop=4 shiftwidth=4 noexpandtab:
