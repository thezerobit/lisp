#ifndef METHOD_H_
#define METHOD_H_

#include "lisp.h"
#include "type.h"

typedef struct {
  void * type;
  const char * name;
  type_assoc * callables;
} method;

typedef method * Method;

Method new_method(const char * name);
Method get_method(void * p);
int is_method(void * p);
void method_add_default(Method m, Callable c);
void method_add_callable(Method m, Type t, Callable c);

#endif /* METHOD_H_ */
