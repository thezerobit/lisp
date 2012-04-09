#include "gc.h"
#include <assert.h>
#include "vector.h"


/* Vector */

int is_vector(pointer p) {
  return get_other(p)->type == TYPE_VECTOR;
}

Vector get_vector(pointer p) {
  assert(is_vector(p));
  return (Vector)p;
}

Vector alloc_vector(int size) {
  Vector v = (Vector)GC_MALLOC(sizeof(vector));
  v->type = TYPE_VECTOR;
  v->count = size;
  v->elems = (pointer *)GC_MALLOC(sizeof(pointer) * size);
  return v;
}

pointer new_vector_from_list(pointer list) {
  assert(is_pair(list));
  Vector vec = alloc_vector(count(list));
  pointer next = list;
  int offset = 0;
  while(is_pair(next)) {
    vec->elems[offset++] = car(next);
    next = cdr(next);
  }
  return (pointer)vec;
}

pointer vector_get(pointer v, pointer offset) {
  assert(is_vector(v));
  assert(is_int(offset));
  int n = get_int(offset);
  return get_vector(v)->elems[n];
}

pointer ff_vector_ref(pointer l) {
  assert(is_pair(l));
  pointer v = car(l);
  pointer rest = cdr(l);
  return vector_get(v, rest);
}

void test_vector() {
  pointer list_of_numbers = read_first("(1 2 3 4 5 6)");
  pointer v = new_vector_from_list(list_of_numbers);
  assert(is_equal(new_int(1), vector_get(v, new_int(0))));
}

