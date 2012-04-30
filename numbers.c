#include <gc.h>
#include "numbers.h"

/* Int */

int is_int(pointer p) {
  return get_type(p) == TYPE_INT;
}

pointer new_int(int64_t num) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  /* printf("creating int: %llu \n", num); */
  o->type = TYPE_INT;
  o->int_num = num;
  return (pointer)o;
}

int64_t get_int(pointer p) {
  check(is_int(p), "get_int: p is not an int");
  return get_other(p)->int_num;
}

int is_int_equal(pointer p, pointer o) {
  Other a = (Other)p;
  Other b = (Other)o;
  return (a->int_num == b->int_num);
}

/* Operations */

pointer ff_plus(pointer args) {
  uint64_t accum = 0;
  pointer next = args;
  while(is_pair(next)) {
    accum += get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_minus(pointer args) {
  uint64_t accum = get_int(car(args));
  pointer next = cdr(args);
  while(is_pair(next)) {
    accum -= get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_mult(pointer args) {
  uint64_t accum = 1;
  pointer next = args;
  while(is_pair(next)) {
    accum *= get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_div(pointer args) {
  uint64_t accum = get_int(car(args));
  pointer next = cdr(args);
  while(is_pair(next)) {
    accum /= get_int(car(next));
    next = cdr(next);
  }
  return new_int(accum);
}

pointer ff_lt(pointer args) {
  uint64_t prev_int = get_int(car(args));
  pointer next = cdr(args);
  uint64_t next_int;
  while(is_pair(next)) {
    next_int = get_int(car(next));
    if(prev_int >= next_int) {
      return BOOLEAN_FALSE;
    }
    prev_int = next_int;
    next = cdr(next);
  }
  return BOOLEAN_TRUE;
}

pointer ff_gt(pointer args) {
  uint64_t prev_int = get_int(car(args));
  pointer next = cdr(args);
  uint64_t next_int;
  while(is_pair(next)) {
    next_int = get_int(car(next));
    if(prev_int <= next_int) {
      return BOOLEAN_FALSE;
    }
    prev_int = next_int;
    next = cdr(next);
  }
  return BOOLEAN_TRUE;
}

pointer ff_lte(pointer args) {
  return not(ff_gt(args));
}

pointer ff_gte(pointer args) {
  return not(ff_lt(args));
}
