/* BOEHM! */
#include "gc.h"
#include "readline/readline.h"
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <glib.h>
#include <gio/gio.h>

#include "persistenthashmap.h"
#include "lisp.h"
#include "env.h"
#include "symbol.h"
#include "keyword.h"
#include "vector.h"
#include "hashmap.h"

/* nil == empty list */

int is_nil (pointer p) {
  return p == NIL;
}

/* Pair */

int is_pair(pointer p) {
  return get_type(p) == TYPE_PAIR;
}

Pair get_pair(pointer p) {
  check(is_pair(p), "Pair expected.");
  return (Pair)p;
}

pointer new_pair(pointer car, pointer cdr) {
  check(car != NULL, "new_pair: car is null.");
  check(cdr != NULL, "new_pair: cdr is null.");
  Pair cell = (Pair)GC_MALLOC(sizeof(pair));
  cell->type = TYPE_PAIR;
  cell->car = car;
  cell->cdr = cdr;
  return (pointer)cell;
}

pointer car(pointer p) {
  Pair pair = get_pair(p);
  return pair->car;
}

pointer cdr(pointer p) {
  Pair pair = get_pair(p);
  return pair->cdr;
}

/* hack for letrec */
pointer set_car(pointer p, pointer val) {
  Pair pair = get_pair(p);
  pair->car = val;
  return pair;
}

pointer reverse(pointer p) {
  pointer last = NIL;
  while(!is_nil(p)) {
    pointer next = new_pair(car(p), last);
    p = cdr(p);
    last = next;
  }
  return last;
}

pointer ff_reverse(pointer p) {
  return reverse(car(p));
}

int count(pointer p) {
  check(is_nil(p) || is_pair(p), "count: p should be pair or nil");
  if(is_nil(p)) {
    return 0;
  }
  int length = 0;
  pointer n;
  n = p;
  do {
    length++;
    n = cdr(n);
  } while (is_pair(n));
  return length;
}

pointer ff_cons(pointer p) {
  return new_pair(car(p), car(cdr(p)));
}

pointer ff_map(pointer p) {
  return NIL;
}

/* Other */

Other get_other(pointer p) {
  return (Other)p;
}

int get_type(pointer p) {
  return ((Other)p)->type;
}

/* Int */

int is_int(pointer p) {
  return get_type(p) == TYPE_INT;
}

pointer new_int(int64_t num) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  /* printf("creating int: %llu \n", num); */
  o->type = TYPE_INT;
  o->int_num = num;
  return (pointer)o;
}

int64_t get_int(pointer p) {
  Other o = (Other)p;
  check(is_int(p), "get_int: p is not an int");
  return get_other(p)->int_num;
}

/* Func */

int is_func(pointer p) {
  return get_type(p) == TYPE_FUNC;
}

pointer new_func(pointer (*f)(pointer)) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_FUNC;
  o->ffunc = f;
  return (pointer)o;
}

pointer call_func(pointer f, pointer arglist) {
  return (*get_other(f)->ffunc)(arglist);
}

void test_func() {
  pointer print = new_func(ff_plus);
  check(is_equal(new_int(10), call_func(print, new_pair(new_int(10), NIL))),
      "test_func: result incorrect");
}

/* String */

int is_string(pointer p) {
  return get_type(p) == TYPE_STRING;
}

pointer new_string(const char * s) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_STRING;
  o->str = s;
  return (pointer)o;
}

const char * get_string(pointer p) {
  check(is_string(p), "get_string: p is not a string");
  return get_other(p)->str;
}

/* Boolean */

int is_boolean(pointer p) {
  return get_type(p) == TYPE_BOOLEAN;
}

pointer new_boolean(int b) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_BOOLEAN;
  o->int_num = (uint64_t)b;
  return (pointer)o;
}

int get_boolean(pointer p) {
  check(is_boolean(p), "get_boolean: p is not boolean");
  return (int)(get_other(p)->int_num);
}

/* Lambda */

int is_lambda(pointer p) {
  return get_type(p) == TYPE_LAMBDA;
}

pointer new_lambda(pointer arglist, pointer body, pointer env) {
  Lambda l = (Lambda)GC_MALLOC(sizeof(lambda));
  l->type = TYPE_LAMBDA;
  l->arglist = arglist;
  l->body = body;
  l->env = env;
  return (pointer)l;
}

Lambda get_lambda(pointer p) {
  check(is_lambda(p), "get_lambda: p is not lambda");
  return (Lambda)p;
}

pointer evaluate_block(pointer body, pointer env) {
  pointer result = NIL;
  int is_tail = 0;
  while(is_pair(body)) {
    result = evaluate(car(body), env);
    body = cdr(body);
  }
  return result;
}

pointer call_lambda(Lambda l, pointer arglist) {
  pointer env, args, argsymbols;
  // build env from arglist
lambda_start:
  env = l->env;
  args = arglist;
  argsymbols = l->arglist;
  while(is_pair(args)) {
    pointer sym = car(argsymbols);
    check(is_symbol(sym), "call_lambda: sym is not symbol");
    /* bind the rest of the args in a list to "... & foo)" */
    if(is_symbol_equal(sym, SYMBOL_AMPERSAND)) {
      argsymbols = cdr(argsymbols);
      sym = car(argsymbols);
      env = add_env(env, sym, args);
      args = NIL;
      argsymbols = cdr(argsymbols);
    } else {
      env = add_env(env, sym, car(args));
      args = cdr(args);
      argsymbols = cdr(argsymbols);
    }
  }
  check(is_nil(argsymbols), "call_lambda: too many symbols");
  check(is_nil(args), "call_lambda: too many arguments");
  /* evaluate forms in new environment, return last or nil
  /* return evaluate_block(l->body, env); */
  pointer body = l->body;
  pointer result = NIL;
  int is_tail = 0;
  while(is_pair(body)) {
    is_tail = !is_pair(cdr(body));
    result = evaluate_inner(car(body), env, is_tail);
    body = cdr(body);
  }
  if(is_boink(result)) {
    Boink b = get_boink(result);
    l = b->l;
    arglist = b->args;
    goto lambda_start;
  }
  return result;
}

/* Boink */

int is_boink(pointer p) {
  return get_type(p) == TYPE_BOINK;
}

pointer new_boink(Lambda l, pointer args) {
  Boink b = (Boink)GC_MALLOC(sizeof(boink));
  b->type = TYPE_BOINK;
  b->l = l;
  b->args = args;
  return (pointer)b;
}

Boink get_boink(pointer p) {
  check(is_boink(p), "get_boink: p is not boink");
  return (Boink)p;
}


/* Let */

/**
 * (a 10 b 20 c 30) -> ((a b c) (10 20 30))
 */
pointer get_letrec_env(pointer defs, pointer env) {
  int c = count(defs);
  check(c % 2 == 0, "get_letrec_env: uneven number of forms");
  while(is_pair(defs)) {
    env = add_env(env, car(defs), NIL);
    defs = cdr(cdr(defs));
  }
  return env;
}

pointer evaluate_let(pointer both, pointer env, pointer which) {
  pointer defs = car(both);
  pointer body = cdr(both);
  int c = count(defs);
  check(c % 2 == 0, "evaluate_let: uneven number of forms");
  pointer new_env = env;
  pointer lh, rh;
  if(is_symbol_equal(which, SYMBOL_LETREC)) {
    new_env = get_letrec_env(defs, env);
  }

  while(is_pair(defs)) {
    lh = car(defs);
    check(is_symbol(lh), "evaluate_let: form is not symbol");
    defs = cdr(defs);
    if(is_symbol_equal(which, SYMBOL_LET_STAR)) {
      rh = evaluate(car(defs), new_env);
      new_env = add_env(new_env, lh, rh);
    }
    if(is_symbol_equal(which, SYMBOL_LET)) {
      rh = evaluate(car(defs), env);
      new_env = add_env(new_env, lh, rh);
    }
    if(is_symbol_equal(which, SYMBOL_LETREC)) {
      rh = evaluate(car(defs), new_env);
      set_env(new_env, lh, rh);
    }
    defs = cdr(defs);
  }
  return evaluate_block(body, new_env);
}

/* Equality */

int is_equal(pointer p, pointer o) {
  Vector v1;
  Vector v2;
  int c, i;
  if(p == o) {
    return 1;
  }
  int p_type = get_type(p);
  int o_type = get_type(o);
  Other a = (Other)p;
  Other b = (Other)o;
  if(p_type != o_type) {
    return 0;
  } else {
    switch(p_type) {
      case TYPE_PAIR:
        return is_equal(car(p), car(o)) && is_equal(cdr(p), cdr(o));
        break;
      case TYPE_NIL:
        /* two nil values should have same pointer value already */
        check(0, "is_equal: two NILs with different value");
        break;
      case TYPE_SYMBOL:
        return is_symbol_equal(p, o);
        break;
      case TYPE_KEYWORD:
        return is_keyword_equal(p, o);
        break;
      case TYPE_INT:
        return (a->int_num == b->int_num);
        break;
      case TYPE_FUNC:
        return (a->ffunc == b->ffunc);
      case TYPE_STRING:
        return strcmp(a->str, b->str) == 0;
      case TYPE_VECTOR:
        v1 = get_vector(p);
        v2 = get_vector(o);
        if(v1->count != v1->count) {
          return 0;
        } else {
          c = v1->count;
          for(i = 0; i < c; ++ i) {
            if(!is_equal(v1->elems[i], v2->elems[i])) {
              return 0;
            }
          }
          return 1;
        }
        break;
      case TYPE_HASHMAP:
        // TODO: compare key/vals
      default:
        return 0;
        break;
    }
  }
  return 0; // make the static analyzers happy
}

void test_is_equal() {
  pointer i1 = new_int(99);
  pointer i2 = new_int(100);
  pointer i3 = new_int(99);
  pointer s1 = new_symbol("foo");
  pointer s2 = new_symbol("bar");
  pointer s3 = new_symbol("foo");
  pointer n1 = NIL;
  pointer n2 = NIL;
  pointer p1 = new_pair(new_int(1), NIL);
  pointer p2 = new_pair(new_int(1), NIL);
  pointer p3 = new_pair(new_int(1), new_pair(new_int(2), NIL));
  pointer p4 = new_pair(new_int(1), new_pair(new_int(2), NIL));
  pointer p5 = new_pair(new_int(1), new_pair(new_int(3), NIL));

  pointer str1 = new_string("hi");
  pointer str2 = new_string("hello");
  pointer str3 = new_string("hello");

  assert( is_equal(i1, i1));
  assert(!is_equal(i1, i2));
  assert( is_equal(i1, i3));
  assert(!is_equal(i2, i3));

  assert( is_equal(s1, s1));
  assert(!is_equal(s1, s2));
  assert( is_equal(s1, s3));
  assert(!is_equal(s2, s3));

  assert(!is_equal(i1, s1));
  assert(!is_equal(i1, n1));
  assert(!is_equal(s1, n1));
  assert(!is_equal(s1, p1));
  assert(!is_equal(i1, p1));
  assert(!is_equal(n1, p1));

  assert( is_equal(n1, n2));

  assert( is_equal(p1, p1));
  assert( is_equal(p1, p2));
  assert(!is_equal(p1, p3));
  assert(!is_equal(p1, p4));
  assert(!is_equal(p1, p5));
  assert( is_equal(p3, p4));
  assert(!is_equal(p4, p5));

  assert(!is_equal(str1, str2));
  assert( is_equal(str3, str2));
}

int hash_thing(pointer p) {
  int t = get_type(p);
  int64_t i;
  switch(t) {
    case TYPE_PAIR:
      // TODO: fix
      return g_direct_hash(p);
    case TYPE_NIL:
      return TYPE_NIL;
    case TYPE_SYMBOL:
      return g_str_hash(get_symbol(p)->name);
    case TYPE_KEYWORD:
      return g_str_hash(get_keyword(p)->name) + 1;
    case TYPE_INT:
      i = get_int(p);
      return (int)(i & 0xFFFF);
    case TYPE_FUNC:
      return g_direct_hash(p);
    case TYPE_STRING:
      return g_str_hash(get_string(p)) + 2;
    case TYPE_VECTOR:
      // TODO: fix
      return g_direct_hash(p);
    case TYPE_BOOLEAN:
      return g_direct_hash(p);
    case TYPE_LAMBDA:
      return g_direct_hash(p);
    case TYPE_MUTABLE_HASH:
      // TODO: fix
      return g_direct_hash(p);
    case TYPE_HASHMAP:
      // TODO: fix
      return g_direct_hash(p);
    default:
      return g_direct_hash(p);
  }
}

/* evaluate */

void init_globals() {
  init_INodes(is_equal, hash_thing); /* PersistentHashMap init */
  NIL = GC_MALLOC(sizeof(int));
  ((Other)NIL)->type = TYPE_NIL;
  SYMBOL_QUOTE = new_symbol("quote");
  SYMBOL_QUASIQUOTE = new_symbol("quasiquote");
  SYMBOL_UNQUOTE = new_symbol("unquote");
  SYMBOL_UNQUOTESPLICING = new_symbol("unquote-splicing");
  SYMBOL_IF = new_symbol("if");
  SYMBOL_TRUE = new_symbol("true");
  SYMBOL_FALSE = new_symbol("false");
  BOOLEAN_TRUE = new_boolean(1);
  BOOLEAN_FALSE = new_boolean(0);
  SYMBOL_LAMBDA = new_symbol("lambda");
  SYMBOL_DEF = new_symbol("def");
  SYMBOL_SYS = new_symbol("sys");
  SYMBOL_LET = new_symbol("let");
  SYMBOL_LET_STAR = new_symbol("let*");
  SYMBOL_LETREC = new_symbol("letrec");
  SYMBOL_DEFMACRO = new_symbol("defmacro");
  SYMBOL_AMPERSAND = new_symbol("&");
}

pointer evaluate_list(pointer args, pointer env) {
  if(is_pair(args)) {
    return new_pair(evaluate(car(args), env), evaluate_list(cdr(args), env));
  } else {
    return NIL;
  }
}

pointer evaluate_vector(pointer v, pointer env) {
  check(is_vector(v), "evaluate_vector: v is not vector");
  Vector old_vec = get_vector(v);
  Vector new_vec = alloc_vector(old_vec->count);
  int i;
  for(i = 0; i < old_vec->count; ++ i) {
    new_vec->elems[i] = evaluate(old_vec->elems[i], env);
  }
  return (pointer)new_vec;
}

typedef struct {
  void * env;
  void * hm;
} hash_builder;

void add_to_hash(pointer key, pointer val, pointer data) {
  hash_builder * hb = (hash_builder *)data;
  void * new_key = evaluate(key, hb->env);
  void * new_val = evaluate(val, hb->env);
  hb->hm = PersistentHashMap_assoc(hb->hm, new_key, new_val);
}

pointer evaluate_hashmap(pointer h, pointer env) {
  HashMap hm = get_hashmap(h);
  HashMap new_hm = new_hashmap();
  hash_builder * hb = (hash_builder *)GC_MALLOC(sizeof(hash_builder));
  hb->env = env;
  hb->hm = PersistentHashMap_EMPTY;
  PersistentHashMap_foreach(hm->phm, add_to_hash, hb);
  new_hm->phm = hb->hm;
  return new_hm;
}

pointer read_first(const char * input) {
  pointer l = read_from_string(input);
  return car(l);
}

pointer evaluate_pair(pointer form, pointer env, int is_tail) {
  pointer first = car(form);
  /* IF */
  if(is_symbol_equal(first, SYMBOL_IF)) {
    int length = count(cdr(form));
    check(length == 2 || length == 3,
        "evaluate_pair: 'if' clause has wrong number of forms");
    pointer second = evaluate(car(cdr(form)), env);
    if(is_nil(second) || second == BOOLEAN_FALSE) {
      if(length > 2) {
        return evaluate_inner(car(cdr(cdr(cdr(form)))), env, is_tail);
      } else {
        return NIL;
      }
    } else {
      return evaluate_inner(car(cdr(cdr(form))), env, is_tail);
    }
  }
  /* QUOTE */
  if(is_symbol_equal(first, SYMBOL_QUOTE)) {
    return car(cdr(form)); // unevaluated
  }
  /* QUASIQUOTE */
  if(is_symbol_equal(first, SYMBOL_QUASIQUOTE)) {
    return quasiquote(car(cdr(form)), env); // unevaluated
  }
  /* LAMBDA */
  if(is_symbol_equal(first, SYMBOL_LAMBDA)) {
    pointer rest = cdr(form);
    return new_lambda(car(rest), cdr(rest), env); // unevaluated
  }
  /* DEF */
  if(is_symbol_equal(first, SYMBOL_DEF)) {
    pointer def_name = car(cdr(form));
    check(is_symbol(def_name), "evaluate_pair: 'def' used with non-symbol");
    pointer def_form = evaluate(car(cdr(cdr(form))), env);
    def_env(env, def_name, def_form);
    return def_form;
  }
  /* SYS */
  if(is_symbol_equal(first, SYMBOL_SYS)) {
    print_thing(env);printf("\n");
    return NIL;
  }
  /* LET */
  if(is_symbol_equal(first, SYMBOL_LET)) {
    return evaluate_let(cdr(form), env, SYMBOL_LET);
  }
  /* LET* */
  if(is_symbol_equal(first, SYMBOL_LET_STAR)) {
    return evaluate_let(cdr(form), env, SYMBOL_LET_STAR);
  }
  /* LETREC */
  if(is_symbol_equal(first, SYMBOL_LETREC)) {
    return evaluate_let(cdr(form), env, SYMBOL_LETREC);
  }
  pointer first_eval = evaluate(car(form), env);
  /* WRAPPED C FUNCTION */
  if(is_func(first_eval)) {
    return call_func(first_eval, evaluate_list(cdr(form), env));
  }
  if(is_lambda(first_eval)) {
    Lambda lam = get_lambda(first_eval);
    pointer args = evaluate_list(cdr(form), env);
    if(is_tail) {
      // TODO: return boink to be captured in call_lambda for tail recursion
      return new_boink(lam, args);
    } else {
      return call_lambda(lam, args);
    }
  }
  /* printf("first: "); print_thing(first); printf("\n"); */
  /* printf("first_eval: "); print_thing(first_eval); printf("\n"); */
  printf("What is this?: "); print_thing(form); printf("\n");
  check(0, "evaluate_pair: form is not special form, or lambda");
  return form;
}

pointer evaluate(pointer form, pointer env) {
  return evaluate_inner(form, env, 0);
}

pointer evaluate_inner(pointer form, pointer env, int is_tail) {
  Other o = get_other(form);
  switch(o->type) {
    case TYPE_PAIR:
      return evaluate_pair(form, env, is_tail);
    case TYPE_SYMBOL:
      return lookup_env(env, form);
    case TYPE_VECTOR:
      return evaluate_vector(form, env);
    case TYPE_HASHMAP:
      return evaluate_hashmap(form, env);
    default:
      return form;
  }
}

pointer quasiquote(pointer form, pointer env) {
  if(is_pair(form)) {
    /* unquote? */
    if(is_symbol_equal(car(form), SYMBOL_UNQUOTE)) {
      return evaluate(car(cdr(form)), env);
    }
    /* quasiquote each element of list */
    pointer result = NIL;
    pointer next = form;
    do {
      pointer elem = car(next);
      if(is_pair(elem) && car(elem) == SYMBOL_UNQUOTESPLICING) {
        pointer sub_list = evaluate(car(cdr(elem)), env);
        check(is_pair(sub_list) || is_nil(sub_list),
            "quasiquote: unquote-splicing non-nil, non-list");
        while(is_pair(sub_list)) {
          result = new_pair(car(sub_list), result);
          sub_list = cdr(sub_list);
        }
      } else {
        result = new_pair(quasiquote(elem, env), result);
      }
      next = cdr(next);
    } while (is_pair(next));
    return reverse(result);
  }
  /* return quoted atom */
  return form;
}

pointer e(char * code, pointer env) {
  return evaluate(read_first(code), env);
}

void test_evaluate() {
  pointer env = build_core_env();

  assert(is_equal(new_symbol("hello"), evaluate(read_first("(quote hello)"), env)));
  assert(is_equal(new_int(100), evaluate(read_first("(if () 200 100)"), env)));
  assert(is_equal(new_int(200), evaluate(read_first("(if 1 200 100)"), env)));
  assert(is_equal(NIL, evaluate(read_first("(if () 200)"), env)));
  assert(is_equal(new_int(42), evaluate(read_first("(+ 2 40)"), env)));
  assert(is_equal(new_int(4200), evaluate(read_first("(* 100 (+ 2 40))"), env)));
  assert(is_equal(new_int(10), evaluate(read_first("(/ 100 (+ 5 5))"), env)));
  assert(is_equal(new_int(20), evaluate(read_first("(- 50 30)"), env)));
  assert(is_equal(new_int(-30), evaluate(read_first("-30"), env)));
  assert(is_equal(new_string("hello"), evaluate(read_first("\"hello\""), env)));
  assert(is_equal(new_string("test \\ slash"), evaluate(read_first("\"test \\\\ slash\""), env)));
  assert(is_equal(new_string("test \\ slash"), evaluate(read_first("\"test \\\\ slash\""), env)));
  assert(is_equal(read_first("[1 2 7]"), evaluate(read_first("[1 2 (+ 3 4)]"), env)));
  assert(is_equal(BOOLEAN_TRUE, evaluate(read_first("true"), env)));
  assert(is_equal(BOOLEAN_FALSE, evaluate(read_first("false"), env)));
  assert(is_equal(new_int(1), evaluate(read_first("(if true 1 0)"), env)));
  assert(is_equal(new_int(0), evaluate(read_first("(if false 1 0)"), env)));
  assert(is_equal(BOOLEAN_TRUE, evaluate(read_first("(> 10 3)"), env)));
  assert(is_equal(BOOLEAN_FALSE, evaluate(read_first("(<= 10 3)"), env)));
  assert(is_equal(new_int(99), evaluate(read_first("((lambda (x) (* x 3)) 33)"), env)));

  assert(is_equal(new_int(10), e("(let (a 10) (let (a 20 b a) b))", env)));
  assert(is_equal(new_int(20), e("(let (a 10) (let* (a 20 b a) b))", env)));
  assert(is_equal(new_int(20), e("(let (a 10) (letrec (a 20 b a) b))", env)));
  assert(is_equal(new_int(20), e("(letrec (a (lambda () b) b 20) (a))", env)));
  assert(is_equal(e("(list 1 2 3)", env), e("(vector->list [1 2 3])", env)));
  assert(is_equal(e("(vector 1 2 3)", env), e("(list->vector (list 1 2 3))", env)));
  // It's hard to test tail recursion without a long pause, since the C stack is
  // pretty resilient. Set the 100 to 1000000 in next line to prove that
  // it is working.
  assert(is_equal(new_int(0), e("(letrec (b (lambda (x) (if (<= x 0) x (b (- x 1))))) (b 100))", env)));
}

/* macros */

pointer macro_pass(pointer forms, pointer env) {
  pointer pforms = NIL;
  pointer n;
  n = forms;
  do {
    pforms = new_pair(macro_pass_each(car(n), env), pforms);
    n = cdr(n);
  } while (is_pair(n));
  pforms = reverse(pforms);
  return pforms;
}

pointer macro_pass_each(pointer form, pointer env) {
  if(is_pair(form)) {
    pointer first = car(form);
    pointer rest = cdr(form);
    if(is_symbol_equal(first, SYMBOL_DEFMACRO)) {
      pointer macro_name = car(rest);
      rest = cdr(rest);
      pointer arglist = car(rest);
      pointer body_forms = cdr(rest);
      pointer macro = new_lambda(arglist, body_forms, env);
      defmacro_env(env, macro_name, macro);
      return NIL;
    } else if(is_symbol(first)) {
      pointer macro = lookup_macro_env(env, first);
      if(macro) {
        check(is_lambda(macro), "macro_pass_each: macro is not lambda");
        return macro_expand_all(macro, rest, env);
      }
    }
  }
  return form;
}

pointer macro_expand_all(pointer macro, pointer body, pointer env) {
  Lambda l = get_lambda(macro);
  /* print_thing(body); printf("\n"); */
  pointer new_form = call_lambda(l, body);
  /* print_thing(new_form);printf("\n"); */
  return macro_pass_each(new_form, env);
}

/* read */

int is_alpha(char c) {
  int ic = (int)c;
  return (ic >= 65 && ic <= 90) || (ic >= 97 && ic <= 122);
}

int is_first_symbol_char(char c) {
  if(is_alpha(c)) {
    return 1;
  }
  return strchr("+*-!/><=?&", c) != NULL;
}

int is_numeric(char c) {
  int ic = (int)c;
  return (ic >= 48 && ic <= 57);
}

int is_alphanumeric(char c) {
  return is_alpha(c) || is_numeric(c);
}

int is_symbol_char(char c) {
  return is_first_symbol_char(c) || is_numeric(c);
}

int is_whitespace(char c) {
  switch(c) {
  case ' ':
  case '\t':
  case '\n':
  case ',':
    return 1;
  default:
    return 0;
  }
}

void skip_whitespace(read_pointer * rp) {
skip_whitey:
  while(is_whitespace(*(rp->loc))) {
    if(*(rp->loc) == '\n') {
      rp->line++;
      rp->col = 1;
    } else {
      rp->col++;
    }
    rp->loc++;
  }
  if(*(rp->loc)==';') {
    do {
      rp->loc++;
      rp->col++;
    } while(*(rp->loc) != '\n');
    goto skip_whitey;
  }
}

int read_required(read_pointer * rp, char c) {
  char actual = *(rp->loc);
  check_rp(actual == c, "read_required: unexpected character found", rp);
  if(actual == c) {
    rp->loc++;
    rp->col++;
    return 1;
  }
  return 0;
}

pointer read_symbol(read_pointer * rp, int keyword) {
  if(keyword) {
    read_required(rp, ':');
  }
  int length = 0;
  const char * start = rp->loc;
  const char * next = start;
  while(is_symbol_char(*next)) {
    length++;
    next++;
  }
  char * name = (char *)GC_MALLOC(length + 1);
  strncpy(name, start, length);
  name[length] = 0;
  rp->loc = next;
  rp->col += length;
  if(keyword) {
    return new_keyword(name);
  }
  return new_symbol(name);
}

pointer read_keyword(read_pointer * rp) {
  return read_symbol(rp, 1);
}

pointer read_number(read_pointer * rp) {
  const char * start = rp->loc;
  const char * next = start;
  if((*next) == '-') {
    next++;
  }
  while(is_numeric(*next)) {
    next++;
  }
  int length = next - start;
  char * name = (char *)GC_MALLOC(next - start + 1);
  strncpy(name, start, length);
  name[length] = 0;
  rp->loc = next;
  rp->col += length;
  int64_t num;
  sscanf(name, "%" SCNd64, &num);
  return new_int(num);
}

pointer read_string(read_pointer * rp) {
  check_rp((*rp->loc) == '"', "read_string: first char is not double-quote", rp);

  const char * next = rp->loc + 1;
  int length = 0;
  int escape_next = 0;
  while(1) {
    check_rp((*next) != 0, "read_string: unexpected null char", rp);
    if(escape_next) {
      next++;
      length++;
      escape_next = 0;
    } else {
      if((*next) == '\\') {
        escape_next = 1;
        next++;
      } else if((*next == '"')) {
        break;
      } else {
        escape_next = 0;
        length++;
        next++;
      }
    }
  }

  char * n_string = (char *)GC_MALLOC(length + 1);

  rp->loc++;
  escape_next = 0;
  char * p_n_string = n_string;
  while(1) {
    check_rp((*rp->loc) != 0, "read_string: unexpected null char", rp);
    if(escape_next) {
      (*p_n_string++) = (*rp->loc++);
      escape_next = 0;
    } else {
      if((*rp->loc) == '\\') {
        escape_next = 1;
        rp->loc++;
      } else if((*rp->loc == '"')) {
        break;
      } else {
        escape_next = 0;
        (*p_n_string++) = (*rp->loc++);
      }
    }
  }
  n_string[length] = 0;
  rp->loc++;

  return new_string(n_string);
}

pointer read_next(read_pointer * rp) {
  char next, succ;
  skip_whitespace(rp);
  next = *(rp->loc);
  switch(next) {
  case 0:
    return NULL;
  case '(':
    return read_pair(rp);
  case '[':
    return read_vec(rp);
  case '{':
    return read_hashmap(rp);
  case '"':
    return read_string(rp);
  case ':':
    return read_keyword(rp);
  case '\'':
    read_required(rp, '\'');
    return new_pair(SYMBOL_QUOTE, new_pair(read_next(rp), NIL));
  case '`':
    read_required(rp, '`');
    return new_pair(SYMBOL_QUASIQUOTE, new_pair(read_next(rp), NIL));
  case '~':
    read_required(rp, '~');
    next = *(rp->loc);
    if(next != '@') {
      return new_pair(SYMBOL_UNQUOTE, new_pair(read_next(rp), NIL));
    } else {
      read_required(rp, '@');
      return new_pair(SYMBOL_UNQUOTESPLICING, new_pair(read_next(rp), NIL));
    }
  default:
    break;
  }
  succ = *(rp->loc + 1);
  if(is_numeric(next) || (next == '-' && is_numeric(succ))) {
    return read_number(rp);
  } else if(is_first_symbol_char(next)) {
    return read_symbol(rp, 0);
  }
  return NULL;
}

pointer read_inside_list(read_pointer * rp) {
  pointer last_pair = NIL;
  pointer next_item;
  /* build a reverse list of elements read */
  while(next_item = read_next(rp)) {
    last_pair = new_pair(next_item, last_pair);
  }
  /* reverse it */
  pointer list = reverse(last_pair);
  return list;
}

pointer read_pair(read_pointer * rp) {
  read_required(rp, '(');
  pointer list = read_inside_list(rp);
  read_required(rp, ')');
  return list;
}

pointer read_vec(read_pointer * rp) {
  read_required(rp, '[');
  pointer list = read_inside_list(rp);
  pointer v = new_vector_from_list(list);
  read_required(rp, ']');
  return v;
}

pointer read_hashmap(read_pointer * rp) {
  read_required(rp, '{');
  pointer list = read_inside_list(rp);
  pointer hm = new_hashmap_from_list(list);
  read_required(rp, '}');
  return hm;
}

/**
 * Reads a series of forms into a list.
 */
pointer read_from_string(const char * input) {
  read_pointer r = {.loc = input, .line = 1, .col = 1};
  read_pointer * rp = &r;
  pointer last_pair = NIL;
  pointer next_item;
  /* build a reverse list of elements read */
  while(1) {
    next_item = read_next(rp);
    if(next_item == NULL) {
      break;
    }
    last_pair = new_pair(next_item, last_pair);
  }
  /* reverse it */
  pointer list = reverse(last_pair);
  return list;
}

/* check */

void check(int cond, const char * desc) {
  if(!cond) {
    fprintf(stderr, "Error: %s.\n", desc);
    assert(0 == "check failed");
  }
}

void check_rp(int cond, const char * desc, read_pointer * rp) {
  if(!cond) {
    if(rp == NULL) {
      fprintf(stderr, "Error: %s.\n", desc);
    } else {
      fprintf(stderr, "Error: %s. Line %d, col %d.", desc, rp->line, rp->col);
    }
    assert(0 == "check failed");
  }
}

/* print */

void print_thing(pointer p) {
  pointer n;
  int i;
  Vector v;
  switch(get_type(p)) {
    case TYPE_PAIR:
      printf("(");
      n = p;
      do {
        print_thing(car(n));
        n = cdr(n);
        if(is_pair(n)) {
          printf(" ");
        }
      } while (is_pair(n));
      printf(")");
      break;
    case TYPE_NIL:
      printf("()");
      break;
    case TYPE_SYMBOL:
      printf("%s", get_symbol(p)->name);
      break;
    case TYPE_KEYWORD:
      printf(":%s", get_symbol(p)->name);
      break;
    case TYPE_INT:
      printf("%" PRId64, get_int(p));
      break;
    case TYPE_FUNC:
      printf("<foreign function>");
      break;
    case TYPE_STRING:
      printf("\"%s\"", get_string(p));
      break;
    case TYPE_VECTOR:
      printf("[");
      v = get_vector(p);
      for(i = 0; i < v->count; ++ i) {
        print_thing(v->elems[i]);
        if(i < v->count - 1) {
          printf(" ");
        }
      }
      printf("]");
      break;
    case TYPE_BOOLEAN:
      if(p == BOOLEAN_TRUE) {
        printf("true");
      } else {
        printf("false");
      }
      break;
    case TYPE_MUTABLE_HASH:
      print_mutable_hash(p);
      break;
    case TYPE_HASHMAP:
      print_hashmap(p);
      break;
    default:
      printf("<unknown object %d>", get_type(p));
      break;
  }
}

pointer ff_print(pointer args) {
  pointer next = args;
  while(1) {
    if(is_pair(next)) {
      print_thing(car(next));
      next = cdr(next);
      if(is_pair(next)) {
        printf(" ");
      }
    } else {
      break;
    }
  }
  return NIL;
}

pointer ff_println(pointer args) {
  pointer ret_val = ff_print(args);
  printf("\n");
  return ret_val;
}

pointer ff_plus(pointer args) {
  uint64_t accum = 0;
  pointer next = args;
  while(is_pair(next)) {
    accum += get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_minus(pointer args) {
  uint64_t accum = get_int(car(args));
  pointer next = cdr(args);
  while(is_pair(next)) {
    accum -= get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_mult(pointer args) {
  uint64_t accum = 1;
  pointer next = args;
  while(is_pair(next)) {
    accum *= get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_div(pointer args) {
  uint64_t accum = get_int(car(args));
  pointer next = cdr(args);
  while(is_pair(next)) {
    accum /= get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_lt(pointer args) {
  uint64_t prev_int = get_int(car(args));
  pointer next = cdr(args);
  uint64_t next_int;
  while(is_pair(next)) {
    next_int = get_int(car(next));
    if(prev_int >= next_int) {
      return BOOLEAN_FALSE;
    }
    prev_int = next_int;
    next = cdr(next);
  }
  return BOOLEAN_TRUE;
}

pointer ff_gt(pointer args) {
  uint64_t prev_int = get_int(car(args));
  pointer next = cdr(args);
  uint64_t next_int;
  while(is_pair(next)) {
    next_int = get_int(car(next));
    if(prev_int <= next_int) {
      return BOOLEAN_FALSE;
    }
    prev_int = next_int;
    next = cdr(next);
  }
  return BOOLEAN_TRUE;
}

pointer not(pointer p) {
  if(is_nil(p) || p == BOOLEAN_FALSE) {
    return BOOLEAN_TRUE;
  }
  return BOOLEAN_FALSE;
}

pointer ff_not(pointer p) {
  return not(car(p));
}

pointer ff_lte(pointer args) {
  return not(ff_gt(args));
}

pointer ff_gte(pointer args) {
  return not(ff_lt(args));
}

pointer ff_list(pointer args) {
  return args;
}

pointer ff_car(pointer args) {
  return car(car(args));
}

pointer ff_cdr(pointer args) {
  return cdr(car(args));
}

/**
 * Equality for any types. All arguments have to be equal to be true.
 */
pointer ff_eq(pointer args) {
  pointer prev = car(args);
  pointer next = cdr(args);
  while(is_pair(next)) {
    if(!is_equal(prev, car(next))) {
      return BOOLEAN_FALSE;
    }
    prev = car(next);
    next = cdr(next);
  }
  return BOOLEAN_TRUE;
}

pointer ff_neq(pointer args) {
  return not(ff_eq(args));
}

/* core env */

pointer build_core_env() {
  pointer env = make_env();
  def_env(env, SYMBOL_TRUE, BOOLEAN_TRUE);
  def_env(env, SYMBOL_FALSE, BOOLEAN_FALSE);
  /* logical */
  def_env(env, new_symbol("not"), new_func(ff_not));
  /* print */
  def_env(env, new_symbol("print"), new_func(ff_print));
  def_env(env, new_symbol("println"), new_func(ff_println));
  /* math */
  def_env(env, new_symbol("+"), new_func(ff_plus));
  def_env(env, new_symbol("-"), new_func(ff_minus));
  def_env(env, new_symbol("*"), new_func(ff_mult));
  def_env(env, new_symbol("/"), new_func(ff_div));
  def_env(env, new_symbol(">"), new_func(ff_gt));
  def_env(env, new_symbol("<"), new_func(ff_lt));
  def_env(env, new_symbol(">="), new_func(ff_gte));
  def_env(env, new_symbol("<="), new_func(ff_lte));
  def_env(env, new_symbol("="), new_func(ff_eq));
  def_env(env, new_symbol("!="), new_func(ff_neq));
  /* list processing */
  def_env(env, new_symbol("list"), new_func(ff_list));
  pointer car_func = new_func(ff_car);
  pointer cdr_func = new_func(ff_cdr);
  def_env(env, new_symbol("car"), car_func);
  def_env(env, new_symbol("first"), car_func);
  def_env(env, new_symbol("cdr"), cdr_func);
  def_env(env, new_symbol("rest"), cdr_func);
  def_env(env, new_symbol("reverse"), new_func(ff_reverse));
  def_env(env, new_symbol("cons"), new_func(ff_cons));
  /* symbols */
  def_env(env, new_symbol("gensym"), new_func(ff_gensym));
  /* vector */
  def_env(env, new_symbol("vector"), new_func(new_vector_from_list));
  def_env(env, new_symbol("vector-ref"), new_func(ff_vector_ref));
  def_env(env, new_symbol("list->vector"), new_func(ff_list_to_vector));
  def_env(env, new_symbol("vector->list"), new_func(ff_vector_to_list));
  /* hashmap */
  def_env(env, new_symbol("get"), new_func(ff_hashmap_get));
  def_env(env, new_symbol("assoc"), new_func(ff_hashmap_assoc));
  def_env(env, new_symbol("hash-map"), new_func(new_hashmap_from_list));
  def_env(env, new_symbol("list->hash-map"), new_func(ff_list_to_hashmap));
  def_env(env, new_symbol("hash-map->list"), new_func(ff_hashmap_to_list));
  /* predicates */
  def_env(env, new_symbol("keyword?"), new_func(ff_is_keyword));

  /* other */
  def_env(env, new_symbol("fib"),
      evaluate(read_first("(lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))"), env));

  load_file("core.lisp", env);


  return env;
}

/* system */

void init_gc() {
  /*
   * Supposedly, this will cause GLIB to do memory allocations
   * in a way that is amenable to Boehm and similar GC's
   */
  putenv("G_SLICE=always-malloc");
  putenv("G_DEBUG=gc-friendly");

  GMemVTable mem;
  memset(&mem, 0, sizeof(GMemVTable));
  mem.malloc  = GC_malloc;
  mem.realloc = GC_realloc;
  mem.free    = GC_free;
  g_mem_set_vtable(&mem);
}

/* the rest */

void load_file(char * filename, pointer env) {
  GFile * gf = g_file_new_for_path(filename);
  char * file_contents;
  gsize file_length;
  gboolean success = g_file_load_contents(gf, 0, &file_contents, &file_length, 0, 0);
  if(success) {
    pointer iforms = read_from_string(file_contents);
    pointer forms = macro_pass(iforms, env);
    while(is_pair(forms)) {
      evaluate(car(forms), env);
      forms = cdr(forms);
    }
  } else {
    printf("unable to load %s\n", filename);
  }
}

void do_repl(pointer env) {
  const char * prompt = ">>> ";
  printf("\nType exit to exit.\n");
  while(1) {
    const char * in;
    void * buf;
    buf = readline(prompt);
    in = (const char *)buf;
    if(in == NULL || strcmp(in, "exit") == 0) {
      break;
    }
    add_history(in);

    pointer iforms = read_from_string(in);
    /* printf("Entered: "); print_thing(iforms); printf("\n"); */
    pointer forms = macro_pass(iforms, env);
    /* printf("After Macro Pass: "); print_thing(forms); printf("\n"); */
    pointer val;
    while(is_pair(forms)) {
      print_thing(evaluate(car(forms), env));
      printf("\n");
      forms = cdr(forms);
    }
    free(buf);
  }

  printf("Goodbye.\n");
}

int main(int argc, char * argv[]) {
  init_gc();
  g_type_init();
  init_symbols();
  init_keywords();
  init_globals();
  test_is_equal();
  test_env();
  test_func();
  test_evaluate();
  test_vector();

  pointer env = build_core_env();

  if(argc > 2) {
    char * one = argv[1];
    char * two = argv[2];
    if(strcmp(one, "--load")==0) {
      load_file(two, env);
      do_repl(env);
    } else if(strcmp(one, "--script")==0) {
      load_file(two, env);
    }
  } else {
    do_repl(env);
  }

  return 0;
}

