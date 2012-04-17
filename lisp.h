#ifndef LISP_H
#define LISP_H

#include <stdint.h>
#include <inttypes.h>
#include <glib.h>

#define TYPE_MASK    (0b011)

#define TYPE_PAIR    0
#define TYPE_NIL     1
#define TYPE_SYMBOL  2
#define TYPE_KEYWORD 3
#define TYPE_INT     4
#define TYPE_FUNC    5
#define TYPE_STRING  6
#define TYPE_VECTOR  7
#define TYPE_BOOLEAN 8
#define TYPE_LAMBDA  9
#define TYPE_MUTABLE_HASH 10
#define TYPE_BOINK   11
#define TYPE_HASHMAP 12

typedef void * pointer;

typedef GHashTable * MutableHash;

/* nil */

pointer NIL;
pointer SYMBOL_QUOTE;
pointer SYMBOL_IF;
pointer SYMBOL_TRUE;
pointer SYMBOL_FALSE;
pointer BOOLEAN_TRUE;
pointer BOOLEAN_FALSE;
pointer SYMBOL_LAMBDA;
pointer SYMBOL_DEF;
pointer SYMBOL_SYS;
pointer SYMBOL_LET;
pointer SYMBOL_LET_STAR;
pointer SYMBOL_LETREC;

int is_nil (pointer p);
pointer new_nil();

/* Pair */

typedef struct {
  int type;
  pointer car;
  pointer cdr;
} pair;

typedef pair * Pair;

int is_pair(pointer p);
Pair get_pair(pointer p);
pointer new_pair(pointer car, pointer cdr);
pointer car(pointer p);
pointer cdr(pointer p);
pointer set_car(pointer p, pointer val);
pointer reverse(pointer p);
int count(pointer p);
pointer ff_map(pointer p);
pointer ff_cons(pointer p);

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

Other get_other(pointer p);

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
pointer eval_block(pointer body, pointer env);
pointer call_lambda(Lambda l, pointer arglist);

/* Boink */

typedef struct {
  int type;
  Lambda l;
  pointer args;
} boink;

typedef boink * Boink;

int is_boink(pointer p);
pointer new_boink(Lambda l, pointer args);
Boink get_boink(pointer p);

/* Let */
pointer let_split(pointer defs);
pointer evaluate_let(pointer both, pointer env, pointer which);

/* Equality */

int is_equal(pointer p, pointer o);
void test_is_equal();
int hash_thing(pointer p);

/* evaluate */

void init_globals();
pointer evaluate_list(pointer args, pointer env);
pointer evaluate_vector(pointer v, pointer env);
pointer evaluate_hashmap(pointer h, pointer env);
pointer read_first(const char * input);
pointer evaluate_pair(pointer form, pointer env, int is_tail);
pointer eval(pointer form, pointer env);
pointer evaluate(pointer form, pointer env);
pointer evaluate_inner(pointer form, pointer env, int is_tail);
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
pointer read_symbol(read_pointer * rp, int keyword);
pointer read_keyword(read_pointer * rp);
pointer read_number(read_pointer * rp);
pointer read_string(read_pointer * rp);
pointer read_next(read_pointer * rp);
pointer read_inside_list(read_pointer * rp);
pointer read_pair(read_pointer * rp);
pointer read_vec(read_pointer * rp);
pointer read_hashmap(read_pointer * rp);
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
void load_file(char * filename, pointer env);

#endif /* LISP_H */
