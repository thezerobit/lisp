#ifndef ENV_H
#define ENV_H

#include "lisp.h"

pointer make_env();
pointer add_env(pointer env, pointer sym, pointer value);
pointer def_env(pointer env, pointer sym, pointer value);
pointer defmacro_env(pointer env, pointer sym, pointer value);

/* terrible hack for letrec */
pointer set_env(pointer env, pointer sym, pointer value);

pointer lookup_env(pointer env, pointer sym);
pointer lookup_macro_env(pointer env, pointer sym);
void test_env();

#endif /* ENV_H */
