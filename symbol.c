#include "gc.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include "symbol.h"

GHashTable* g_symbols;

void init_symbols() {
  g_symbols = g_hash_table_new(g_str_hash, g_str_equal);
}

int is_symbol(pointer p) {
  return (TYPE_MASK & (uint64_t)p) == TYPE_SYMBOL;
}

pointer new_symbol(char * name) {
  pointer found = g_hash_table_lookup(g_symbols, name);
  if(found) {
    return found;
  }
  Symbol sym = (Symbol)GC_MALLOC(sizeof(struct symbol));
  sym->name = name;
  pointer new_s = (pointer)((uint64_t)sym | TYPE_SYMBOL);
  g_hash_table_insert(g_symbols, name, new_s);
  return new_s;
}

Symbol get_symbol(pointer p) {
  return (Symbol)((uint64_t)p & ~(uint64_t)TYPE_MASK);
}

int is_symbol_equal(pointer p, pointer o) {
  return p == o;
}
