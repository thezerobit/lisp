#include <stdio.h>
#include <assert.h>
#include "gc.h"
#include "env.h"
#include "symbol.h"
#include "mutable_hash.h"
#include "numbers.h"

/* Inner Env */

typedef struct {
  pointer defs;
  pointer macros;
} inner_env;

pointer new_inner_env() {
  inner_env * nenv = (inner_env *)GC_MALLOC(sizeof(inner_env));
  nenv->defs = new_mutable_hash();
  nenv->macros = new_mutable_hash();
  return nenv;
}

void inner_env_def(pointer e, pointer key, pointer val) {
  inner_env * ie = (inner_env *)e;
  mutable_hash_set(ie->defs, key, val);
}

pointer inner_env_def_lookup(pointer e, pointer key) {
  inner_env * ie = (inner_env *)e;
  return mutable_hash_get(ie->defs, key);
}

void inner_env_defmacro(pointer e, pointer key, pointer val) {
  inner_env * ie = (inner_env *)e;
  mutable_hash_set(ie->macros, key, val);
}

pointer inner_env_defmacro_lookup(pointer e, pointer key) {
  inner_env * ie = (inner_env *)e;
  return mutable_hash_get(ie->macros, key);
}

/* Env */

pointer make_env() {
  return new_pair(
      NIL,
      new_pair(
        new_inner_env(),
        NIL));
}

pointer add_env(pointer env, pointer sym, pointer value) {
  /* printf("A: "); print_thing(env); printf("\n"); */
  pointer current_scope = car(env);
  pointer rest = cdr(env);
  pointer map = new_pair(sym, new_pair(value, NIL));
  /* printf("map: "); print_thing(map); printf("\n"); */
  pointer new_current_scope = new_pair(map, current_scope);
  /* printf("new_current_scope: "); print_thing(new_current_scope); printf("\n"); */
  return new_pair(new_current_scope, rest);
}

pointer def_env(pointer env, pointer sym, pointer value) {
  pointer inner_env = car(cdr(env));
  inner_env_def(inner_env, sym, value);
  return env;
}

pointer defmacro_env(pointer env, pointer sym, pointer value) {
  pointer inner_env = car(cdr(env));
  inner_env_defmacro(inner_env, sym, value);
  return env;
}

/* terrible hack for letrec */
pointer set_env(pointer env, pointer sym, pointer value) {
  pointer scope = car(env);
  while(is_pair(scope)) {
    pointer binding = car(scope);
    pointer symbol_found = car(binding);
    if(is_symbol_equal(symbol_found, sym)) {
      pointer old_val = car(cdr(binding));
      assert(old_val == NIL /* letrec shouldn't repeat bindings */);
      set_car(cdr(binding), value);
      return env;
    }
    scope = cdr(scope);
  }
  assert(0);
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
  pointer inner_env = car(cdr(env));
  found = inner_env_def_lookup(inner_env, sym);
  if(found != NULL) {
    return found;
  }
  printf("symbol not found: %s\n", get_symbol(sym)->name);
  assert(0);
  return NULL;
}

pointer lookup_macro_env(pointer env, pointer sym) {
  pointer inner_env = car(cdr(env));
  return inner_env_defmacro_lookup(inner_env, sym);
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

