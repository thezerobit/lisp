#include <gc.h>
#include "method.h"
#include "type.h"

Method new_method(const char * name) {
  Method m = (Method)GC_MALLOC(sizeof(method));
  m->type = TYPE_METHOD;
  m->name = name;
  m->callables = new_type_assoc();
  return m;
}

Method get_method(void * p) {
  return (Method)p;
}

int is_method(void * p) {
  return get_type(p) == TYPE_METHOD;
}

void method_add_default(Method m, Callable c) {
  default_type_assoc(m->callables, c);
}

void method_add_callable(Method m, Type t, Callable c) {
  set_type_assoc(m->callables, t, c);
}

Callable method_get_callable(Method m, Type t) {
  return get_type_assoc(m->callables, t);
}
