#ifndef NUMBERS_H_
#define NUMBERS_H_

#include "lisp.h"

/* Int */

int is_int(pointer p);
pointer new_int(int64_t num);
int64_t get_int(pointer p);
int is_int_equal(pointer p, pointer o);

/* Operations */

pointer ff_plus(pointer args);
pointer ff_minus(pointer args);
pointer ff_mult(pointer args);
pointer ff_div(pointer args);
pointer ff_lt(pointer args);
pointer ff_gt(pointer args);
pointer ff_lte(pointer args);
pointer ff_gte(pointer args);

#endif /* NUMBERS_H_ */
