#ifndef SYMBOL_H
#define SYMBOL_H

#include "lisp.h"

typedef struct {
  int type;
  const char * name;
} symbol;

typedef symbol * Symbol;

void init_symbols();
int is_symbol(pointer p);
pointer new_symbol(const char * name);
Symbol get_symbol(pointer p);
int is_symbol_equal(pointer p, pointer o);
pointer gensym(const char * prefix);
pointer ff_gensym(pointer p);

#endif /* SYMBOL_H */
