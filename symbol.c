#include "gc.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "symbol.h"

GHashTable* g_symbols;
int gensym_counter = 1;

void init_symbols() {
  g_symbols = g_hash_table_new(g_str_hash, g_str_equal);
}

int is_symbol(pointer p) {
  return get_symbol(p)->type == TYPE_SYMBOL;
}

pointer new_symbol(const char * name) {
  pointer found = g_hash_table_lookup(g_symbols, name);
  if(found) {
    return found;
  }
  Symbol sym = (Symbol)GC_MALLOC(sizeof(symbol));
  sym->type = TYPE_SYMBOL;
  sym->name = name;
  pointer new_s = (pointer)sym;
  g_hash_table_insert(g_symbols, (gpointer)name, new_s);
  return new_s;
}

Symbol get_symbol(pointer p) {
  return (Symbol)p;
}

int is_symbol_equal(pointer p, pointer o) {
  return p == o;
}

pointer gensym(const char * prefix) {
  char temp[128];
  sprintf(temp, "%s_%d", prefix, gensym_counter++);
  int length = strlen(temp);
  char * new_string = (char *)GC_MALLOC(length+1);
  strcpy(new_string, temp);
  return new_symbol(new_string);
}

pointer ff_gensym(pointer p) {
  if(is_nil(p)) {
    return gensym("_gsym_");
  }
  pointer name = car(p);
  if(is_string(name)){
    const char * prefix = get_string(name);
    return gensym(prefix);
  } else if(is_symbol(name)) {
    const char * prefix = get_symbol(name)->name;
    return gensym(prefix);
  }
  return NIL;
}
