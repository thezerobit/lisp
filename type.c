#include <gc.h>
#include "type.h"

int type_counter = 0;

pointer new_type(const char * name) {
  slang_type * t = (slang_type *)GC_MALLOC(sizeof(slang_type));
  t->name = name;
  t->ord = ++type_counter;
  return (pointer)t;
}
