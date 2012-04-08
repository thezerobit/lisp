#ifndef LISP_H
#define LISP_H

#include <stdint.h>
#include <inttypes.h>
#include <glib.h>

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
#define TYPE_MUTABLE_HASH 10

typedef void * pointer;

typedef GHashTable * MutableHash;

/* nil */

int is_nil (pointer p);
pointer new_nil();

/* Pair */

struct pair {
  pointer car;
  pointer cdr;
};

typedef struct pair * Pair;

int is_pair(pointer p);
Pair get_pair(pointer p);
pointer new_pair(pointer car, pointer cdr);
pointer car(pointer p);
pointer cdr(pointer p);
pointer reverse(pointer p);
int count(pointer p);

/* Other */

typedef struct {
  int type;
  union {
    void * data;
    int64_t int_num;
    pointer (*ffunc)(pointer);
    const char * str;
    MutableHash mut_hash;
  };
} other;

typedef other * Other;

int is_other(pointer p);
Other get_other(pointer p);
int is_other_equal(pointer p, pointer o);

/* Int */

int is_int(pointer p);
pointer new_int(int64_t num);
int64_t get_int(pointer p);

/* Func */

int is_func(pointer p);
pointer new_func(pointer (*f)(pointer));
pointer call_func(pointer f, pointer arglist);
void test_func();

/* String */

int is_string(pointer p);
pointer new_string(const char * s);
const char * get_string(pointer p);

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

/* Boolean */

int is_boolean(pointer p);
pointer new_boolean(int b);
int get_boolean(pointer p);

/* Lambda */

typedef struct {
  int type;
  pointer arglist;
  pointer body;
  pointer env;
} lambda;

typedef lambda * Lambda;

int is_lambda(pointer p);
pointer new_lambda(pointer arglist, pointer body, pointer env);
Lambda get_lambda(pointer p);
pointer call_lambda(Lambda l, pointer arglist);

/* Equality */

int is_equal(pointer p, pointer o);
void test_is_equal();

/* evaluate */

void init_globals();
pointer evaluate_list(pointer args, pointer env);
pointer evaluate_vector(pointer v, pointer env);
pointer read_first(const char * input);
pointer evaluate(pointer form, pointer env);
void test_evaluate();

/* read */

typedef struct {
  const char * loc;
  int line;
  int col;
} read_pointer;

int is_alpha(char c);
int is_first_symbol_char(char c);
int is_numeric(char c);
int is_alphanumeric(char c);
int is_symbol_char(char c);
int is_whitespace(char c);
void skip_whitespace(read_pointer * rp);
int read_required(read_pointer * rp, char c);
pointer read_symbol(read_pointer * rp);
pointer read_number(read_pointer * rp);
pointer read_string(read_pointer * rp);
pointer read_next(read_pointer * rp);
pointer read_pair(read_pointer * rp);
pointer read_vec(read_pointer * rp);
pointer read_from_string(const char * input);

/* print */

void print_thing(pointer p);
pointer ff_print(pointer args);
pointer ff_println(pointer args);
pointer ff_plus(pointer args);
pointer ff_minus(pointer args);
pointer ff_mult(pointer args);
pointer ff_div(pointer args);
pointer ff_lt(pointer args);
pointer ff_gt(pointer args);
pointer not(pointer p);
pointer ff_lte(pointer args);
pointer ff_gte(pointer args);
pointer ff_eq(pointer args);
pointer ff_neq(pointer args);

/* core env */

pointer build_core_env();

/* system */

void init_gc();

#endif /* LISP_H */
