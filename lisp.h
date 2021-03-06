#ifndef LISP_H
#define LISP_H

#include <stdint.h>
#include <inttypes.h>
#include <glib.h>


typedef void * pointer;

pointer TYPE_PAIR;
pointer TYPE_NIL;
pointer TYPE_SYMBOL;
pointer TYPE_KEYWORD;
pointer TYPE_INT;
pointer TYPE_FUNC;
pointer TYPE_STRING;
pointer TYPE_VECTOR;
pointer TYPE_BOOLEAN;
pointer TYPE_LAMBDA;
pointer TYPE_MUTABLE_HASH;
pointer TYPE_BOINK;
pointer TYPE_HASHMAP;
pointer TYPE_CALLABLE;
pointer TYPE_METHOD;

typedef GHashTable * MutableHash;

void init_types();

/* nil */

pointer NIL;
pointer SYMBOL_QUOTE;
pointer SYMBOL_QUASIQUOTE;
pointer SYMBOL_UNQUOTE;
pointer SYMBOL_UNQUOTESPLICING;
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
pointer SYMBOL_DEFMACRO;
pointer SYMBOL_AMPERSAND;

int is_nil (pointer p);
int is_pointer_equal(pointer p, pointer o);
pointer new_nil();

/* Pair */

typedef struct {
  void * type;
  pointer car;
  pointer cdr;
} pair;

typedef pair * Pair;

int is_pair(pointer p);
int is_pair_equal(pointer p, pointer o);
Pair get_pair(pointer p);
pointer new_pair(pointer car, pointer cdr);
pointer car(pointer p);
pointer cdr(pointer p);
pointer set_car(pointer p, pointer val);
pointer reverse(pointer p);
pointer ff_reverse(pointer p);
int count(pointer p);
pointer ff_map(pointer p);
pointer ff_cons(pointer p);

/* Other */

typedef struct {
  void * type;
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
pointer get_type(pointer p);
const char * get_type_name(pointer p);

/* Func */

int is_func(pointer p);
pointer new_func(pointer (*f)(pointer));
pointer call_func(pointer f, pointer arglist);
void test_func();

/* String */

int is_string(pointer p);
pointer new_string(const char * s);
const char * get_string(pointer p);
int is_string_equal(pointer p, pointer o);

/* Boolean */

int is_boolean(pointer p);
pointer new_boolean(int b);
int get_boolean(pointer p);

/* Lambda */

typedef struct {
  void * type;
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
  void * type;
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
pointer quasiquote(pointer form, pointer env);
void test_evaluate();

/* macros */

pointer macro_pass(pointer form, pointer env);
pointer macro_pass_each(pointer form, pointer env);
pointer macro_expand_all(pointer macro, pointer body, pointer env);

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

/* check */

void check(int cond, const char * desc);
void check_rp(int cond, const char * desc, read_pointer * rp);

/* print */

void print_thing(pointer p);
pointer ff_print(pointer args);
pointer ff_println(pointer args);
pointer not(pointer p);
pointer ff_not(pointer p);
pointer ff_list(pointer args);
pointer ff_car(pointer args);
pointer ff_cdr(pointer args);

/* Equality */

pointer ff_eq(pointer args);
pointer ff_neq(pointer args);

/* core env */

pointer build_core_env();

/* system */

void init_gc();
void load_file(char * filename, pointer env);

/* callable */

#define CALLABLE_LAMBDA 1
#define CALLABLE_FFUNC  2

typedef struct {
  void * type;
  int callable_type;
  union {
    Lambda lam;
    pointer (*ffunc)(pointer);
  };
} callable;

typedef callable * Callable;

Callable new_callable(int t, void * p);
pointer invoke_callable(Callable c, pointer args);

#endif /* LISP_H */
