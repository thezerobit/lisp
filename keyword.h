#ifndef KEYWORD_H
#define KEYWORD_H

#include "lisp.h"

typedef struct {
  int type;
  const char * name;
} keyword;

typedef keyword * Keyword;

void init_keywords();
int is_keyword(pointer p);
pointer new_keyword(const char * name);
Keyword get_keyword(pointer p);
int is_keyword_equal(pointer p, pointer o);
pointer ff_is_keyword(pointer p);

#endif /* KEYWORD_H */

