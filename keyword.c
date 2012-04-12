#include "gc.h"
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include "keyword.h"

GHashTable* g_keywords;

void init_keywords() {
  g_keywords = g_hash_table_new(g_str_hash, g_str_equal);
}

int is_keyword(pointer p) {
  return get_keyword(p)->type == TYPE_KEYWORD;
}

pointer new_keyword(const char * name) {
  pointer found = g_hash_table_lookup(g_keywords, name);
  if(found) {
    return found;
  }
  Keyword sym = (Keyword)GC_MALLOC(sizeof(keyword));
  sym->type = TYPE_KEYWORD;
  sym->name = name;
  pointer new_s = (pointer)sym;
  g_hash_table_insert(g_keywords, (gpointer)name, new_s);
  return new_s;
}

Keyword get_keyword(pointer p) {
  return (Keyword)p;
}

int is_keyword_equal(pointer p, pointer o) {
  return p == o;
}

pointer ff_is_keyword(pointer p) {
  assert(cdr(p) == NIL);
  if(is_keyword(car(p))) {
    return BOOLEAN_TRUE;
  }
  return BOOLEAN_FALSE;
}

