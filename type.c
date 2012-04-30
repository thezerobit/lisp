#include <gc.h>
#include "type.h"

int type_counter = 0;

pointer new_type(const char * name) {
  slang_type * t = (slang_type *)GC_MALLOC(sizeof(slang_type));
  t->name = name;
  t->ord = ++type_counter;
  return (pointer)t;
}

int hibit(unsigned int n) {
  n |= (n >>  1);
  n |= (n >>  2);
  n |= (n >>  4);
  n |= (n >>  8);
  n |= (n >> 16);
  return n - (n >> 1);
}

int min_pow_2(int in) {
  int hi = hibit((unsigned int)in);
  return 1 << hi;
}

type_assoc * new_type_assoc() {
  type_assoc * ta = (type_assoc *)GC_MALLOC(sizeof(type_assoc));
  int l = min_pow_2(type_counter);
  ta->length = l;
  ta->data = (void **)GC_MALLOC(sizeof(void *) * l);
  ta->def = NULL;
  return ta;
}

void set_type_assoc(type_assoc * ta, Type t, void * val) {
  int ord = t->ord;
  if(ord >= ta->length) {
    int l = min_pow_2(type_counter);
    ta->data = (void **)GC_REALLOC(ta->data, sizeof(void *) * l);
  }
  ta->data[ord] = val;
}

void default_type_assoc(type_assoc * ta, void * val) {
  ta->def = val;
}

void * get_type_assoc(type_assoc * ta, Type t) {
  int ord = t->ord;
  if(ord >= ta->length) {
    return NULL;
  }
  void * result = ta->data[ord];
  if(result == NULL) {
    result = ta->def;
  }
  return result;
}
