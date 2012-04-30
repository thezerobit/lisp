#ifndef TYPE_H_
#define TYPE_H_

#include "lisp.h"

typedef struct {
  int ord;
  const char * name;
} slang_type;

typedef struct {
  int length;
  void ** data;
  void * def;
} type_assoc;

typedef slang_type * Type;

pointer new_type(const char * name);
pointer get_builtin_type(int t);

type_assoc * new_type_assoc();
void set_type_assoc(type_assoc * ta, Type t, void * val);
void default_type_assoc(type_assoc * ta, void * val);
void * get_type_assoc(type_assoc * ta, Type t);

#endif /* TYPE_H_ */
