/* BOEHM! */
#include "gc.h"
#include "readline/readline.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

#define TYPE_MASK    (0b011)

#define TYPE_PAIR    (0b000)
#define TYPE_NIL     (0b001)
#define TYPE_SYMBOL  (0b010)
#define TYPE_OTHER   (0b011)

#define TYPE_INT     4
#define TYPE_FUNC    5
#define TYPE_STRING  6
#define TYPE_VECTOR  7
#define TYPE_BOOLEAN 8
#define TYPE_LAMBDA  9
#define TYPE_REF     10

typedef void * pointer;

/* advanced declarations */

void print_thing(pointer p);
pointer ff_print(pointer args);

pointer build_core_env();
pointer read_first(const char * input);
pointer read_from_string(const char * input);
const char * get_string(pointer p);
int is_lambda(pointer p);
pointer add_env(pointer env, pointer sym, pointer value);
pointer evaluate(pointer form, pointer env);
pointer evaluate_list(pointer args, pointer env);

/* nil == empty list */

int is_nil (pointer p) {
  return (TYPE_MASK & (uint64_t)p) == TYPE_NIL;
}

pointer new_nil() {
  return (pointer)((uint64_t)NULL | TYPE_NIL);
}

/* Pair */

struct pair {
  pointer car;
  pointer cdr;
};

typedef struct pair * Pair;

int is_pair(pointer p) {
  return p && (TYPE_MASK & (uint64_t)p) == TYPE_PAIR;
}

Pair get_pair(pointer p) {
  assert(p);
  assert(is_pair(p));
  return (Pair)p;
}

pointer new_pair(pointer car, pointer cdr) {
  assert(car);
  assert(cdr);
  pointer p = GC_MALLOC(sizeof(struct pair));
  Pair cell = get_pair(p);
  cell->car = car;
  cell->cdr = cdr;
  return p;
}

pointer car(pointer p) {
  Pair pair = get_pair(p);
  return pair->car;
}

pointer cdr(pointer p) {
  Pair pair = get_pair(p);
  return pair->cdr;
}

pointer reverse(pointer p) {
  pointer last = new_nil();
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

/* Symbol */

struct symbol {
  char * name;
};

typedef struct symbol * Symbol;

int is_symbol(pointer p) {
  return (TYPE_MASK & (uint64_t)p) == TYPE_SYMBOL;
}

pointer new_symbol(char * name) {
  Symbol sym = (Symbol)GC_MALLOC(sizeof(struct symbol));
  /* sym->name = GC_MALLOC(strlen(name) + 1); */
  /* strcpy(sym->name, name); */
  sym->name = name;
  return (pointer)((uint64_t)sym | TYPE_SYMBOL);
}

Symbol get_symbol(pointer p) {
  return (Symbol)((uint64_t)p & ~(uint64_t)TYPE_MASK);
}

int is_symbol_equal(pointer p, pointer o) {
  return strcmp(get_symbol(p)->name, get_symbol(o)->name) == 0;
}

/* Other */

typedef struct {
  int count;
  pointer * elems;
} vector;

typedef vector * Vector;

typedef struct {
  pointer arglist;
  pointer body;
  pointer env;
} lambda;

typedef lambda * Lambda;

typedef struct {
  int type;
  union {
    void * data;
    int64_t int_num;
    pointer (*ffunc)(pointer);
    const char * str;
    Vector vec;
    Lambda lambda;
    pointer object; // for Ref type
  };
} other;

typedef other * Other;

int is_other(pointer p) {
  return (TYPE_MASK & (uint64_t)p) == TYPE_OTHER;
}

Other get_other(pointer p) {
  return (Other)((uint64_t)p & ~(uint64_t)TYPE_MASK);
}

int is_other_equal(pointer p, pointer o) {
  Other a = get_other(p);
  Other b = get_other(o);
  Vector v1, v2;
  int i, c;
  if(a->type != b->type) {
    return 0;
  } else {
    switch(a->type) {
      case TYPE_INT:
        return (a->int_num == b->int_num);
        break;
      case TYPE_FUNC:
        return (a->ffunc == b->ffunc);
      case TYPE_STRING:
        return strcmp(a->str, b->str) == 0;
      case TYPE_VECTOR:
        v1 = a->vec;
        v2 = b->vec;
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
    }
  }
}

/* Int */

int is_int(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_INT;
}

pointer new_int(int64_t num) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  /* printf("creating int: %llu \n", num); */
  o->type = TYPE_INT;
  o->int_num = num;
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

int64_t get_int(pointer p) {
  assert(is_int(p));
  return get_other(p)->int_num;
}

/* Func */

int is_func(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_FUNC;
}

pointer new_func(pointer (*f)(pointer)) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_FUNC;
  o->ffunc = f;
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

pointer call_func(pointer f, pointer arglist) {
  return (*get_other(f)->ffunc)(arglist);
}

void test_func() {
  pointer print = new_func(ff_print);
  printf("Should print (): ");
  call_func(print, new_pair(new_nil(), new_nil()));
  printf("\n");
}

/* String */

int is_string(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_STRING;
}

pointer new_string(const char * s) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_STRING;
  o->str = s;
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

const char * get_string(pointer p) {
  assert(is_string(p));
  return get_other(p)->str;
}

/* Vector */

int is_vector(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_VECTOR;
}

Vector get_vector(pointer p) {
  assert(is_vector(p));
  return get_other(p)->vec;
}

Other alloc_vector(int size) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_VECTOR;
  o->vec = (Vector)GC_MALLOC(sizeof(vector));
  o->vec->count = size;
  o->vec->elems = (pointer *)GC_MALLOC(sizeof(pointer) * size);
  return o;
}

pointer new_vector_from_list(pointer list) {
  assert(is_pair(list));
  Other o = alloc_vector(count(list));
  pointer next = list;
  int offset = 0;
  while(is_pair(next)) {
    o->vec->elems[offset++] = car(next);
    next = cdr(next);
  }
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

pointer vector_get(pointer v, pointer offset) {
  assert(is_vector(v));
  assert(is_int(offset));
  int n = get_int(offset);
  return get_vector(v)->elems[n];
}

pointer ff_vector_ref(pointer l) {
  assert(is_pair(l));
  pointer v = car(l);
  pointer rest = cdr(l);
  return vector_get(v, rest);
}

void test_vector() {
  pointer list_of_numbers = read_first("(1 2 3 4 5 6)");
  pointer v = new_vector_from_list(list_of_numbers);
  assert(is_equal(new_int(1), vector_get(v, new_int(0))));
  print_thing(v);
  printf("\n");
}

/* Boolean */

int is_boolean(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_BOOLEAN;
}

pointer new_boolean(int b) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_BOOLEAN;
  o->int_num = (uint64_t)b;
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

int get_boolean(pointer p) {
  assert(is_boolean(p));
  return (int)(get_other(p)->int_num);
}

/* Lambda */

int is_lambda(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_LAMBDA;
}

pointer new_lambda(pointer arglist, pointer body, pointer env) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_LAMBDA;
  o->lambda = (Lambda)GC_MALLOC(sizeof(lambda));
  Lambda l = o->lambda;
  l->arglist = arglist;
  l->body = body;
  l->env = env;
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

Lambda get_lambda(pointer p) {
  assert(is_lambda(p));
  return get_other(p)->lambda;
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
  pointer result = new_nil();
  pointer body = l->body;
  while(is_pair(body)) {
    result = evaluate(car(body), env);
    body = cdr(body);
  }
  return result;
}

/* Ref */

int is_ref(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_REF;
}

pointer new_ref(pointer obj) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  o->type = TYPE_REF;
  o->object = obj;
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

/* Equality */

int is_equal(pointer p, pointer o) {
  if(p == o) {
    return 1;
  } else if((TYPE_MASK & (uint64_t)p) != (TYPE_MASK & (uint64_t)o)) {
    return 0;
  } else {
    switch(TYPE_MASK & (uint64_t)p) {
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
      case TYPE_OTHER:
      default:
        return is_other_equal(p, o);
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
  pointer n1 = new_nil();
  pointer n2 = new_nil();
  pointer p1 = new_pair(new_int(1), new_nil());
  pointer p2 = new_pair(new_int(1), new_nil());
  pointer p3 = new_pair(new_int(1), new_pair(new_int(2), new_nil()));
  pointer p4 = new_pair(new_int(1), new_pair(new_int(2), new_nil()));
  pointer p5 = new_pair(new_int(1), new_pair(new_int(3), new_nil()));

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

/* functional map
 *
 * First rendition will just be built with lists, 'cause I'm lazy.
 */

pointer new_map() {
  return new_nil();
}

pointer assoc(pointer map, pointer sym, pointer value) {
  return new_pair(new_pair(sym, new_pair(value, new_nil())), map);
}

pointer dissoc(pointer map, pointer sym) {
  return assoc(map, sym, new_nil());
}

pointer get(pointer map, pointer sym) {
  if(is_pair(map)) {
    pointer first = car(map);
    if(is_equal(car(first), sym)) {
      return car(cdr(first));
    } else {
      return get(cdr(map), sym);
    }
  } else {
    return new_nil();
  }
}

void test_map() {
  pointer m1 = new_map();
  pointer m2 = assoc(m1, new_symbol("foo"), new_int(100));
  pointer m3 = assoc(m2, new_int(2), new_int(200));
  pointer m4 = assoc(m2, new_symbol("foo"), new_int(300));
  assert(is_equal(new_int(100), get(m2, new_symbol("foo"))));
  assert(is_equal(new_int(200), get(m3, new_int(2))));
  assert(is_equal(new_int(100), get(m3, new_symbol("foo"))));
  assert(is_equal(new_int(300), get(m4, new_symbol("foo"))));
  assert(is_equal(new_nil(),    get(m4, new_symbol("baz"))));
}

/* env */

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

/* evaluate */

pointer SYMBOL_QUOTE;
pointer SYMBOL_IF;
pointer SYMBOL_TRUE;
pointer SYMBOL_FALSE;
pointer BOOLEAN_TRUE;
pointer BOOLEAN_FALSE;
pointer SYMBOL_LAMBDA;
pointer SYMBOL_DEF;
pointer SYMBOL_REF;
pointer SYMBOL_DEREF;
pointer SYMBOL_REFSET;

void init_globals() {
  SYMBOL_QUOTE = new_symbol("quote");
  SYMBOL_IF = new_symbol("if");
  SYMBOL_TRUE = new_symbol("true");
  SYMBOL_FALSE = new_symbol("false");
  BOOLEAN_TRUE = new_boolean(1);
  BOOLEAN_FALSE = new_boolean(0);
  SYMBOL_LAMBDA = new_symbol("lambda");
  SYMBOL_DEF = new_symbol("def");
  SYMBOL_REF = new_symbol("ref");
  SYMBOL_DEREF = new_symbol("deref");
  SYMBOL_REFSET = new_symbol("set!");
}

pointer evaluate_list(pointer args, pointer env) {
  if(is_pair(args)) {
    return new_pair(evaluate(car(args), env), evaluate_list(cdr(args), env));
  } else {
    return new_nil();
  }
}

pointer evaluate_vector(pointer v, pointer env) {
  assert(is_vector(v));
  Vector old_vec = get_vector(v);
  Other o = alloc_vector(old_vec->count);
  Vector new_vec = o->vec;
  int i;
  for(i = 0; i < old_vec->count; ++ i) {
    new_vec->elems[i] = evaluate(old_vec->elems[i], env);
  }
  return (pointer)((uint64_t)o | TYPE_OTHER);
}

pointer read_first(const char * input) {
  pointer l = read_from_string(input);
  return car(l);
}

pointer evaluate(pointer form, pointer env) {
  /* printf("evaluating: "); print_thing(form); printf("\n"); */
  if(is_symbol(form)) {
    return lookup_env(env, form);
  } else if(is_nil(form)) {
    return form;
  } else if(is_other(form)) {
    Other o_form = get_other(form);
    switch(o_form->type) {
      case TYPE_VECTOR:
        return evaluate_vector(form, env);
        break;
      default:
        return form;
    }
  } else { /* pair */
    pointer first = car(form);
    /* IF */
    if(is_equal(first, SYMBOL_IF)) {
      int length = count(cdr(form));
      assert(length == 2 || length == 3);
      pointer second = evaluate(car(cdr(form)), env);
      if(is_nil(second) || second == BOOLEAN_FALSE) {
        if(length > 2) {
          return evaluate(car(cdr(cdr(cdr(form)))), env);
        } else {
          return new_nil();
        }
      } else {
        return evaluate(car(cdr(cdr(form))), env);
      }
    }
    /* QUOTE */
    if(is_equal(first, SYMBOL_QUOTE)) {
      return car(cdr(form)); // unevaluated
    }
    /* LAMBDA */
    if(is_equal(first, SYMBOL_LAMBDA)) {
      pointer rest = cdr(form);
      return new_lambda(car(rest), cdr(rest), env); // unevaluated
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
    return form;
  }
}

void test_evaluate() {
  pointer env = build_core_env();
  pointer forms = read_from_string("(print 100)");
  printf("Should print 100: ");
  evaluate(car(forms), env);
  printf("\n");

  assert(is_equal(new_symbol("hello"), evaluate(read_first("(quote hello)"), env)));
  assert(is_equal(new_int(100), evaluate(read_first("(if () 200 100)"), env)));
  assert(is_equal(new_int(200), evaluate(read_first("(if 1 200 100)"), env)));
  assert(is_equal(new_nil(), evaluate(read_first("(if () 200)"), env)));
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
}

/* read */

typedef struct {
  const char * loc;
  int line;
  int col;
} read_pointer;

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

pointer read_pair(read_pointer * rp);
pointer read_vec(read_pointer * rp);

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
  pointer last_pair = new_nil();
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
  pointer last_pair = new_nil();
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
  pointer last_pair = new_nil();
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
  if(is_pair(p)) {
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
  } else if(is_symbol(p)) {
    printf("%s", get_symbol(p)->name);
  } else if(is_nil(p)) {
    printf("()");
  } else if(is_other(p)) {
    switch(get_other(p)->type) {
      case TYPE_INT:
        printf("%" PRId64, get_int(p));
        break;
      case TYPE_FUNC:
        printf("<foreign function>", get_string(p));
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
      default:
        printf("<unknown object>");
        break;
    }
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
  return new_nil();
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
  env = add_env(env, SYMBOL_TRUE, BOOLEAN_TRUE);
  env = add_env(env, SYMBOL_FALSE, BOOLEAN_FALSE);
  env = add_env(env, new_symbol("print"), new_func(ff_print));
  env = add_env(env, new_symbol("println"), new_func(ff_println));
  env = add_env(env, new_symbol("+"), new_func(ff_plus));
  env = add_env(env, new_symbol("-"), new_func(ff_minus));
  env = add_env(env, new_symbol("*"), new_func(ff_mult));
  env = add_env(env, new_symbol("/"), new_func(ff_div));
  env = add_env(env, new_symbol(">"), new_func(ff_gt));
  env = add_env(env, new_symbol("<"), new_func(ff_lt));
  env = add_env(env, new_symbol(">="), new_func(ff_gte));
  env = add_env(env, new_symbol("<="), new_func(ff_lte));
  env = add_env(env, new_symbol("="), new_func(ff_eq));
  env = add_env(env, new_symbol("!="), new_func(ff_neq));
  env = add_env(env, new_symbol("vector"), new_func(new_vector_from_list));
  env = add_env(env, new_symbol("vector-ref"), new_func(ff_vector_ref));
  return env;
}

/* the rest */

int main() {

  init_globals();
  test_is_equal();
  test_map();
  test_env();
  test_func();
  test_evaluate();
  test_vector();
  printf("Test, success!\n");
  printf("\nType exit to exit.\n");

  const char * prompt = ">>> ";

  while(1) {
    const char * in;
    in = readline(prompt);
    if(strcmp(in, "exit") == 0) {
      break;
    }
    /* printf("You typed: %s\n", in); */
    pointer env = build_core_env();

    pointer forms = read_from_string(in);
    pointer val;
    while(is_pair(forms)) {
      print_thing(evaluate(car(forms), env));
      printf("\n");
      forms = cdr(forms);
    }
  }

  printf("Goodbye.\n");

  return 0;
}

