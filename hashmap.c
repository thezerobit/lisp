#include "gc.h"
#include <assert.h>
#include <stdio.h>
#include "hashmap.h"

/* HashMap */

int is_hashmap(pointer p) {
  return get_type(p) == TYPE_HASHMAP;
}

HashMap get_hashmap(pointer p) {
  assert(is_hashmap(p));
  return (HashMap)p;
}

pointer new_hashmap() {
  HashMap new_hm = (HashMap)GC_MALLOC(sizeof(HashMap));
  new_hm->type = TYPE_HASHMAP;
  new_hm->phm = PersistentHashMap_EMPTY;
  return new_hm;
}

pointer new_hashmap_from_list(pointer list) {
  assert(count(list) % 2 == 0);
  HashMap new_hm = new_hashmap();
  /* TODO: use TransientHashMap */
  PersistentHashMap * phm = new_hm->phm;
  pointer next = list;
  while(is_pair(next)) {
    pointer key = car(next);
    next = cdr(next);
    pointer val = car(next);
    next = cdr(next);
    phm = PersistentHashMap_assoc(phm, key, val);
  }
  new_hm->phm = phm;
  return new_hm;
}

typedef struct {
  pointer list;
} list_builder;

void add_to_list(pointer key, pointer val, pointer data) {
  list_builder * plb = (list_builder *)data;
  plb->list = new_pair(val, new_pair(key, plb->list));
}

pointer list_from_hashmap(pointer h) {
  list_builder lb;
  lb.list = NIL;
  HashMap hm = get_hashmap(h);
  PersistentHashMap_foreach(hm->phm, add_to_list, &lb);
  pointer new_list = reverse(lb.list);
  return new_list;
}

pointer hashmap_get(pointer m, pointer key, pointer notFound) {
  HashMap hm = get_hashmap(m);
  return PersistentHashMap_valAtDef(hm->phm, key, notFound);
}

pointer ff_hashmap_assoc(pointer l) {
  assert(count(l) == 3);
  pointer m = car(l);
  l = cdr(l);
  pointer key = car(l);
  l = cdr(l);
  pointer val = car(l);
  HashMap hm = get_hashmap(m);
  HashMap new_hm = new_hashmap();
  new_hm->phm = PersistentHashMap_assoc(hm->phm, key, val);
  return new_hm;
}

void print_hashmap(pointer p) {
  void * l = list_from_hashmap(p);
  void * n;
  printf("{");
  n = l;
  do {
    print_thing(car(n));
    printf(" ");
    n = cdr(n);
    print_thing(car(n));
    n = cdr(n);
    if(is_pair(n)) {
      printf(", ");
    }
  } while (is_pair(n));
  printf("}");
}

pointer ff_hashmap_get(pointer l) {
  int length = count(l);
  assert(length == 2 || length == 3);
  if(length == 2) {
    return hashmap_get(car(l), car(cdr(l)), NIL);
  } else { /* length == 3 */
    return hashmap_get(car(l), car(cdr(l)), car(cdr(cdr(l))));
  }
}

pointer ff_list_to_hashmap(pointer l) {
  assert(count(l) == 1);
  return new_hashmap_from_list(car(l));
}

pointer ff_hashmap_to_list(pointer l) {
  assert(count(l) == 1);
  return list_from_hashmap(car(l));
}


