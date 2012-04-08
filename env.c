#include <stdio.h>
#include <assert.h>
#include "env.h"
#include "symbol.h"
#include "mutable_hash.h"

pointer make_env() {
  return new_pair(
      new_nil(),
      new_pair(
        new_mutable_hash(),
        new_nil()));
}

pointer add_env(pointer env, pointer sym, pointer value) {
  /* printf("A: "); print_thing(env); printf("\n"); */
  pointer current_scope = car(env);
  pointer rest = cdr(env);
  pointer map = new_pair(sym, new_pair(value, new_nil()));
  /* printf("map: "); print_thing(map); printf("\n"); */
  pointer new_current_scope = new_pair(map, current_scope);
  /* printf("new_current_scope: "); print_thing(new_current_scope); printf("\n"); */
  return new_pair(new_current_scope, rest);
}

pointer def_env(pointer env, pointer sym, pointer value) {
  pointer hash = car(cdr(env));
  mutable_hash_set(hash, sym, value);
  return env;
}

pointer lookup_scope(pointer scope, pointer sym) {
  if(is_pair(scope)) {
    pointer first = car(scope);
    if(is_symbol_equal(car(first), sym)) {
      return car(cdr(first));
    } else {
      return lookup_scope(cdr(scope), sym);
    }
  } else {
    return NULL;
  }
}

pointer lookup_env(pointer env, pointer sym) {
  pointer scope = car(env);
  pointer found = lookup_scope(scope, sym);
  if(found != NULL) {
    return found;
  }
  pointer hash = car(cdr(env));
  found = mutable_hash_get(hash, sym);
  if(found != NULL) {
    return found;
  }
  printf("symbol not found: %s\n", get_symbol(sym)->name);
  assert(0);
  return NULL;
}

void test_env() {
  pointer e5 = make_env();
  e5 = add_env(e5, new_symbol("bar"), new_int(200));
  e5 = add_env(e5, new_symbol("baz"), new_int(200));
  def_env(e5, new_symbol("foo"), new_int(10));
  assert(is_equal(new_int(10), lookup_env(e5, new_symbol("foo"))));
  def_env(e5, new_symbol("foo"), new_int(20));
  assert(is_equal(new_int(20), lookup_env(e5, new_symbol("foo"))));

  pointer e1 = make_env();
  pointer e2 = add_env(e1, new_symbol("foo"), new_int(100));
  pointer e3 = add_env(e2, new_symbol("bar"), new_int(200));
  pointer e4 = add_env(e2, new_symbol("foo"), new_int(300));
  assert(is_equal(new_int(100), lookup_env(e2, new_symbol("foo"))));
  assert(is_equal(new_int(200), lookup_env(e3, new_symbol("bar"))));
  assert(is_equal(new_int(100), lookup_env(e3, new_symbol("foo"))));
  assert(is_equal(new_int(300), lookup_env(e4, new_symbol("foo"))));

}

