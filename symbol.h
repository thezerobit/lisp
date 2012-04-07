#ifndef SYMBOL_H
#define SYMBOL_H

#include "lisp.h"

struct symbol {
  char * name;
};

typedef struct symbol * Symbol;

int is_symbol(pointer p);
pointer new_symbol(char * name);
Symbol get_symbol(pointer p);
int is_symbol_equal(pointer p, pointer o);

#endif /* SYMBOL_H */
