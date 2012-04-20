#ifndef HASHMAP_H
#define HASHMAP_H

#include "lisp.h"
#include "persistenthashmap.h"

/* Map */

typedef struct {
  int type;
  PersistentHashMap * phm;
} hashmap;

typedef hashmap * HashMap;

int is_hashmap(pointer p);
HashMap get_hashmap(pointer p);
pointer new_hashmap();
pointer new_hashmap_from_list(pointer list);
pointer list_from_hashmap(pointer hashmap);
pointer hashmap_get(pointer m, pointer key, pointer notFound);
pointer ff_hashmap_assoc(pointer l);
void print_hashmap(pointer p);
pointer ff_hashmap_get(pointer l);
pointer ff_list_to_hashmap(pointer l);
pointer ff_hashmap_to_list(pointer l);
/* pointer ff_hashmap_to_list(pointer l); */
/* void test_hashmap(); */

#endif /* HASHMAP_H */

