CC=gcc
CFLAGS=-I.

lisp: lisp.o env.o
	$(CC) -o lisp lisp.o env.o -I. -lgc -lreadline

clean:
	rm -f *.o

# vim: set tabstop=4 shiftwidth=4 noexpandtab:
