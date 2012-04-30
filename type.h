#ifndef TYPE_H_
#define TYPE_H_

#include "lisp.h"

typedef struct {
  int ord;
  const char * name;
} slang_type;

typedef slang_type * Type;

pointer new_type(const char * name);
pointer get_builtin_type(int t);

#endif /* TYPE_H_ */
