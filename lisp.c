/* BOEHM! */
#include "gc.h"
#include "readline/readline.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include "glib.h"
#include <gio/gio.h>

#include "lisp.h"
#include "env.h"
#include "symbol.h"
#include "vector.h"

/* nil == empty list */

int is_nil (pointer p) {
  return p == NIL;
}

/* Pair */

int is_pair(pointer p) {
  return get_other(p)->type == TYPE_PAIR;
}

Pair get_pair(pointer p) {
  assert(is_pair(p));
  return (Pair)p;
}

pointer new_pair(pointer car, pointer cdr) {
  assert(car);
  assert(cdr);
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

int count(pointer p) {
  assert(is_nil(p) || is_pair(p));
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

/* Int */

int is_int(pointer p) {
  return get_other(p)->type == TYPE_INT;
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
  assert(is_int(p));
  return get_other(p)->int_num;
}

/* Func */

int is_func(pointer p) {
  return get_other(p)->type == TYPE_FUNC;
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
  assert(is_equal(new_int(10), call_func(print, new_pair(new_int(10), NIL))));
}

/* String */

int is_string(pointer p) {
  return get_other(p)->type == TYPE_STRING;
}

pointer new_string(const char * s) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_STRING;
  o->str = s;
  return (pointer)o;
}

const char * get_string(pointer p) {
  assert(is_string(p));
  return get_other(p)->str;
}

/* Boolean */

int is_boolean(pointer p) {
  return get_other(p)->type == TYPE_BOOLEAN;
}

pointer new_boolean(int b) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_BOOLEAN;
  o->int_num = (uint64_t)b;
  return (pointer)o;
}

int get_boolean(pointer p) {
  assert(is_boolean(p));
  return (int)(get_other(p)->int_num);
}

/* Lambda */

int is_lambda(pointer p) {
  return get_other(p)->type == TYPE_LAMBDA;
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
  assert(is_lambda(p));
  return (Lambda)p;
}

pointer evaluate_block(pointer body, pointer env) {
  pointer result = NIL;
  while(is_pair(body)) {
    result = evaluate(car(body), env);
    body = cdr(body);
  }
  return result;
}

pointer call_lambda(Lambda l, pointer arglist) {
  // build env from arglist
  assert(count(arglist) == count(l->arglist));
  pointer env = l->env;
  pointer args = arglist;
  pointer argsymbols = l->arglist;
  while(is_pair(args)) {
    pointer sym = car(argsymbols);
    assert(is_symbol(sym));
    env = add_env(env, sym, car(args));
    args = cdr(args);
    argsymbols = cdr(argsymbols);
  }
  // evaluate forms in new environment, return last or nil
  return evaluate_block(l->body, env);
}

/* Let */

/**
 * (a 10 b 20 c 30) -> ((a b c) (10 20 30))
 */
pointer get_letrec_env(pointer defs, pointer env) {
  int c = count(defs);
  assert(c % 2 == 0);
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
  assert(c % 2 == 0);
  pointer new_env = env;
  pointer lh, rh;
  if(is_symbol_equal(which, SYMBOL_LETREC)) {
    new_env = get_letrec_env(defs, env);
  }

  while(is_pair(defs)) {
    lh = car(defs);
    assert(is_symbol(lh));
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
  Other a = (Other)p;
  Other b = (Other)o;
  if(a->type != b->type) {
    return 0;
  } else {
    switch(a->type) {
      case TYPE_PAIR:
        return is_equal(car(p), car(o)) && is_equal(cdr(p), cdr(o));
        break;
      case TYPE_NIL:
        /* two nil values should have same pointer value already */
        assert(0);
        break;
      case TYPE_SYMBOL:
        return is_symbol_equal(p, o);
        break;
      case TYPE_INT:
        return (a->int_num == b->int_num);
        break;
      case TYPE_FUNC:
        return (a->ffunc == b->ffunc);
      case TYPE_STRING:
        return strcmp(a->str, b->str) == 0;
      case TYPE_VECTOR:
        v1 = (Vector)a;
        v2 = (Vector)b;
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
      default:
        return 0;
        break;
    }
  }
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

/* evaluate */

void init_globals() {
  NIL = GC_MALLOC(sizeof(int));
  ((Other)NIL)->type = TYPE_NIL;
  SYMBOL_QUOTE = new_symbol("quote");
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
}

pointer evaluate_list(pointer args, pointer env) {
  if(is_pair(args)) {
    return new_pair(evaluate(car(args), env), evaluate_list(cdr(args), env));
  } else {
    return NIL;
  }
}

pointer evaluate_vector(pointer v, pointer env) {
  assert(is_vector(v));
  Vector old_vec = get_vector(v);
  Vector new_vec = alloc_vector(old_vec->count);
  int i;
  for(i = 0; i < old_vec->count; ++ i) {
    new_vec->elems[i] = evaluate(old_vec->elems[i], env);
  }
  return (pointer)new_vec;
}

pointer read_first(const char * input) {
  pointer l = read_from_string(input);
  return car(l);
}

pointer evaluate_pair(pointer form, pointer env) {
  pointer first = car(form);
  /* IF */
  if(is_symbol_equal(first, SYMBOL_IF)) {
    int length = count(cdr(form));
    assert(length == 2 || length == 3);
    pointer second = evaluate(car(cdr(form)), env);
    if(is_nil(second) || second == BOOLEAN_FALSE) {
      if(length > 2) {
        return evaluate(car(cdr(cdr(cdr(form)))), env);
      } else {
        return NIL;
      }
    } else {
      return evaluate(car(cdr(cdr(form))), env);
    }
  }
  /* QUOTE */
  if(is_symbol_equal(first, SYMBOL_QUOTE)) {
    return car(cdr(form)); // unevaluated
  }
  /* LAMBDA */
  if(is_symbol_equal(first, SYMBOL_LAMBDA)) {
    pointer rest = cdr(form);
    return new_lambda(car(rest), cdr(rest), env); // unevaluated
  }
  /* DEF */
  if(is_symbol_equal(first, SYMBOL_DEF)) {
    pointer def_name = car(cdr(form));
    assert(is_symbol(def_name));
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
    return call_lambda(get_lambda(first_eval), evaluate_list(cdr(form), env));
  }
  /* TODO: macros */
  /* printf("first: "); print_thing(first); printf("\n"); */
  /* printf("first_eval: "); print_thing(first_eval); printf("\n"); */
  /* printf("What is this?: "); print_thing(form); printf("\n"); */
  assert(0);
  return form;
}

pointer evaluate(pointer form, pointer env) {
  Other o = get_other(form);
  switch(o->type) {
    case TYPE_PAIR:
      return evaluate_pair(form, env);
    case TYPE_SYMBOL:
      return lookup_env(env, form);
    case TYPE_VECTOR:
      return evaluate_vector(form, env);
    default:
      return form;
  }
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
  return strchr("+*-!/><=", c) != NULL;
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
  return (c == ' ') || (c == '\t') || (c == '\n');
}

void skip_whitespace(read_pointer * rp) {
  while(is_whitespace(*(rp->loc))) {
    if(*(rp->loc) == '\n') {
      rp->line++;
      rp->col = 1;
    } else {
      rp->col++;
    }
    rp->loc++;
  }
}

int read_required(read_pointer * rp, char c) {
  char actual = *(rp->loc);
  assert(actual == c);
  if(actual == c) {
    rp->loc++;
    rp->col++;
    return 1;
  }
  return 0;
}

pointer read_symbol(read_pointer * rp) {
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
  return new_symbol(name);
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
  assert((*rp->loc) == '"');

  const char * next = rp->loc + 1;
  int length = 0;
  int escape_next = 0;
  while(1) {
    assert((*next) != 0);
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
    assert((*rp->loc) != 0);
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
  skip_whitespace(rp);
  char next = *(rp->loc);
  if(next == 0) {
    return NULL;
  }
  char succ = *(rp->loc + 1);
  if(next == '(') {
    return read_pair(rp);
  } else if(next == '[') {
    return read_vec(rp);
  } else if(is_numeric(next) || (next == '-' && is_numeric(succ))) {
    return read_number(rp);
  } else if(is_first_symbol_char(next)) {
    return read_symbol(rp);
  } else if(next == '"') {
    return read_string(rp);
  }
  return NULL;
}

pointer read_pair(read_pointer * rp) {
  read_required(rp, '(');
  pointer last_pair = NIL;
  pointer next_item;
  /* build a reverse list of elements read */
  while(next_item = read_next(rp)) {
    last_pair = new_pair(next_item, last_pair);
  }
  /* reverse it */
  pointer list = reverse(last_pair);
  read_required(rp, ')');
  return list;
}

pointer read_vec(read_pointer * rp) {
  read_required(rp, '[');
  pointer last_pair = NIL;
  pointer next_item;
  /* build a reverse list of elements read */
  while(next_item = read_next(rp)) {
    last_pair = new_pair(next_item, last_pair);
  }
  /* reverse it */
  pointer list = reverse(last_pair);
  pointer v = new_vector_from_list(list);
  read_required(rp, ']');
  return v;
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

/* print */

void print_thing(pointer p) {
  pointer n;
  int i;
  Vector v;
  switch(get_other(p)->type) {
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
    default:
      printf("<unknown object %d>", get_other(p)->type);
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
  def_env(env, new_symbol("cons"), new_func(ff_cons));
  /* vector */
  def_env(env, new_symbol("vector"), new_func(new_vector_from_list));
  def_env(env, new_symbol("vector-ref"), new_func(ff_vector_ref));
  def_env(env, new_symbol("list->vector"), new_func(ff_list_to_vector));
  def_env(env, new_symbol("vector->list"), new_func(ff_vector_to_list));
  /* other */
  def_env(env, new_symbol("fib"),
      evaluate(read_first("(lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))"), env));
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
    pointer forms = read_from_string(file_contents);
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
    in = readline(prompt);
    if(strcmp(in, "exit") == 0) {
      break;
    }

    pointer forms = read_from_string(in);
    pointer val;
    while(is_pair(forms)) {
      print_thing(evaluate(car(forms), env));
      printf("\n");
      forms = cdr(forms);
    }
  }

  printf("Goodbye.\n");
}

int main(int argc, char * argv[]) {
  init_gc();
  g_type_init();
  init_symbols();
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

