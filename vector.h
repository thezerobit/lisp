#ifndef VECTOR_H
#define VECTOR_H

#include "lisp.h"

/* Vector */

typedef struct {
  void * type;
  int count;
  pointer * elems;
} vector;

typedef vector * Vector;

int is_vector(pointer p);
Vector get_vector(pointer p);
Vector alloc_vector(int size);
pointer new_vector_from_list(pointer list);
pointer vector_get(pointer v, pointer offset);
pointer ff_vector_ref(pointer l);
pointer ff_list_to_vector(pointer l);
pointer ff_vector_to_list(pointer l);
void test_vector();

#endif /* VECTOR_H */

