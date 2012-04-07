#include "gc.h"
#include <stdio.h>
#include <assert.h>
#include "symbol.h"

int is_symbol(pointer p) {
  return (TYPE_MASK & (uint64_t)p) == TYPE_SYMBOL;
}

pointer new_symbol(char * name) {
  Symbol sym = (Symbol)GC_MALLOC(sizeof(struct symbol));
  /* sym->name = GC_MALLOC(strlen(name) + 1); */
  /* strcpy(sym->name, name); */
  sym->name = name;
  return (pointer)((uint64_t)sym | TYPE_SYMBOL);
}

Symbol get_symbol(pointer p) {
  return (Symbol)((uint64_t)p & ~(uint64_t)TYPE_MASK);
}

int is_symbol_equal(pointer p, pointer o) {
  return strcmp(get_symbol(p)->name, get_symbol(o)->name) == 0;
}
