#include "gc.h"
#include <stdio.h>
#include <glib.h>
#include "lisp.h"

guint object_hash(gconstpointer gp) {
  pointer p = (pointer)gp;
  if(is_symbol(p)) {
    return g_direct_hash(gp);
  }
  // TODO: call other hash functions based on type
  return g_direct_hash(gp);
}

gboolean object_compare(gconstpointer v1, gconstpointer v2) {
  pointer p1 = (pointer)v1;
  pointer p2 = (pointer)v2;
  return is_equal(p1, p2);
}

/* void mutable_hash_finalize(void * obj, void * cd) { */
/*   Other o = get_other((pointer)obj); */
/*   g_hash_table_destroy(o->mut_hash); */
/* } */

int is_mutable_hash(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_MUTABLE_HASH;
}

pointer new_mutable_hash() {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_MUTABLE_HASH;
  o->mut_hash = g_hash_table_new(object_hash, object_compare);
  // tell Glib to free the hash table when this object gets collected
  /* GC_REGISTER_FINALIZER(o->mut_hash, mutable_hash_finalize, 0, 0, 0); */
  return (pointer)o;
}

MutableHash get_mutable_hash(pointer p) {
  return get_other(p)->mut_hash;
}

int is_mutable_hash_equal(pointer p, pointer o) {
  return p == o; /* identity */
  /* TODO: finish full comparison */
  /* MutableHash a = get_mutable_hash(p); */
  /* MutableHash b = get_mutable_hash(o); */
  /* if(g_hash_table_size(a) != g_hash_table_size(b)) { */
  /*   return 0; */
  /* } */
}

pointer mutable_hash_get(pointer hash, pointer key) {
  return g_hash_table_lookup(get_mutable_hash(hash), key);
}

pointer mutable_hash_set(pointer hash, pointer key, pointer val) {
  g_hash_table_insert(get_mutable_hash(hash), key, val);
  return val;
}

void print_vals(pointer key, pointer val, pointer extra) {
  print_thing(key); printf(" : "); print_thing(val);printf(" ");
}

void print_mutable_hash(pointer p) {
  printf("<mutable_hash {");
  g_hash_table_foreach(get_mutable_hash(p), print_vals, NULL);
}
