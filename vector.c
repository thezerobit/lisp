#include "gc.h"
#include <assert.h>
#include "vector.h"
#include "numbers.h"


/* Vector */

int is_vector(pointer p) {
  return get_type(p) == TYPE_VECTOR;
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

int is_vector_equal(pointer p, pointer o) {
  Vector v1;
  Vector v2;
  int c, i;
  v1 = get_vector(p);
  v2 = get_vector(o);
  if(v1->count != v1->count) {
    return 0;
  }
  c = v1->count;
  for(i = 0; i < c; ++ i) {
    if(!is_equal(v1->elems[i], v2->elems[i])) {
      return 0;
    }
  }
  return 1;
}

pointer ff_vector_ref(pointer l) {
  assert(is_pair(l));
  pointer v = car(l);
  pointer rest = cdr(l);
  return vector_get(v, rest);
}

pointer ff_list_to_vector(pointer l) {
  return new_vector_from_list(car(l));
}

pointer ff_vector_to_list(pointer l) {
  Vector vec = get_vector(car(l));
  pointer new_list = NIL;
  int i;
  for(i = vec->count - 1; i >= 0; -- i) {
    new_list = new_pair(vec->elems[i], new_list);
  }
  return new_list;
}

void test_vector() {
  pointer list_of_numbers = read_first("(1 2 3 4 5 6)");
  pointer v = new_vector_from_list(list_of_numbers);
  assert(is_equal(new_int(1), vector_get(v, new_int(0))));
}

