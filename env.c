#include <stdio.h>
#include <assert.h>
#include "env.h"
#include "symbol.h"

pointer make_env() {
  return new_nil();
}

pointer add_env(pointer env, pointer sym, pointer value) {
  return new_pair(new_pair(sym, new_pair(value, new_nil())), env);
}

pointer lookup_env(pointer env, pointer sym) {
  if(is_pair(env)) {
    pointer first = car(env);
    if(is_symbol_equal(car(first), sym)) {
      return car(cdr(first));
    } else {
      return lookup_env(cdr(env), sym);
    }
  } else {
    printf("symbol not found: %s\n", get_symbol(sym)->name);
    assert(0);
    return NULL;
  }
}

void test_env() {
  pointer e1 = make_env();
  pointer e2 = add_env(e1, new_symbol("foo"), new_int(100));
  pointer e3 = add_env(e2, new_symbol("bar"), new_int(200));
  pointer e4 = add_env(e2, new_symbol("foo"), new_int(300));
  assert(is_equal(new_int(100), lookup_env(e2, new_symbol("foo"))));
  assert(is_equal(new_int(200), lookup_env(e3, new_symbol("bar"))));
  assert(is_equal(new_int(100), lookup_env(e3, new_symbol("foo"))));
  assert(is_equal(new_int(300), lookup_env(e4, new_symbol("foo"))));
}

