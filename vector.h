#ifndef VECTOR_H
#define VECTOR_H

#include "lisp.h"

/* Vector */

typedef struct {
  int type;
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
void test_vector();

#endif /* VECTOR_H */

