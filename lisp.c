/* BOEHM! */
#include "gc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

const TYPE_MASK   = 0b011;

const TYPE_PAIR   = 0b000;
const TYPE_NIL    = 0b001;
const TYPE_SYMBOL = 0b010;
const TYPE_OTHER  = 0b011;

const TYPE_INT    = 4;

typedef void * pointer;
typedef unsigned long long uint64;
typedef signed long long int64;

/* nil == empty list */

int is_nil (pointer p) {
  return (TYPE_MASK & (uint64)p) == TYPE_NIL;
}

pointer new_nil() {
  return (pointer)((uint64)NULL | TYPE_NIL);
}

/* Pair */

struct pair {
  pointer car;
  pointer cdr;
};

typedef struct pair * Pair;

int is_pair(pointer p) {
  return p && (TYPE_MASK & (uint64)p) == TYPE_PAIR;
}

Pair get_pair(pointer p) {
  assert(p);
  assert(is_pair(p));
  return (Pair)p;
}

pointer new_pair(pointer car, pointer cdr) {
  assert(car);
  assert(cdr);
  pointer p = GC_MALLOC(sizeof(struct pair));
  Pair cell = get_pair(p);
  cell->car = car;
  cell->cdr = cdr;
  return p;
}

pointer car(pointer p) {
  Pair pair = get_pair(p);
  return pair->car;
}

pointer cdr(pointer p) {
  Pair pair = get_pair(p);
  return pair->cdr;
}

void set_car(pointer p, pointer thing) {
  Pair pair = get_pair(p);
  pair->car = thing;
}

void set_cdr(pointer p, pointer thing) {
  Pair pair = get_pair(p);
  pair->cdr = thing;
}

pointer reverse(pointer p) {
  pointer last = new_nil();
  while(!is_nil(p)) {
    pointer next = new_pair(car(p), last);
    p = cdr(p);
    last = next;
  }
  return last;
}

int count(pointer p) {
  assert(is_nil(p) || is_pair(p));
  if(is_nil(p)) {
    return 0;
  }
  int length = 0;
  pointer n;
  n = p;
  do {
    length++;
    n = cdr(n);
  } while (is_pair(n));
  return length;
}

/* Symbol */

struct symbol {
  char * name;
};

typedef struct symbol * Symbol;

int is_symbol(pointer p) {
  return (TYPE_MASK & (uint64)p) == TYPE_SYMBOL;
}

pointer new_symbol(char * name) {
  Symbol sym = (Symbol)GC_MALLOC(sizeof(struct symbol));
  /* sym->name = GC_MALLOC(strlen(name) + 1); */
  /* strcpy(sym->name, name); */
  sym->name = name;
  return (pointer)((uint64)sym | TYPE_SYMBOL);
}

Symbol get_symbol(pointer p) {
  return (Symbol)((uint64)p & ~(uint64)TYPE_MASK);
}

/* Other */

typedef struct {
  int type;
  union {
    void * data;
    int64 int_num;
  };
} other;

typedef other * Other;

int is_other(pointer p) {
  return (TYPE_MASK & (uint64)p) == TYPE_OTHER;
}

Other get_other(pointer p) {
  return (Other)((uint64)p & ~(uint64)TYPE_MASK);
}

/* Int */

int is_int(pointer p) {
  return is_other(p) && get_other(p)->type == TYPE_INT;
}

pointer new_int(int64 num) {
  Other o = (Other)GC_MALLOC(sizeof(other));
  /* printf("creating int: %llu \n", num); */
  o->type = TYPE_INT;
  o->int_num = num;
  return (pointer)((uint64)o | TYPE_OTHER);
}

int64 get_int(pointer p) {
  assert(is_int);
  return get_other(p)->int_num;
}

/* read */

typedef struct {
  const char * loc;
  int line;
  int col;
} read_pointer;

int is_alpha(char c) {
  int ic = (int)c;
  return (ic >= 65 && ic <= 90) || (ic >= 97 && ic <= 122);
}

int is_numeric(char c) {
  int ic = (int)c;
  return (ic >= 48 && ic <= 57);
}

int is_alphanumeric(char c) {
  return is_alpha(c) || is_numeric(c);
}

int is_whitespace(char c) {
  return (c == ' ') || (c == '\t') || (c == '\n');
}

void skip_whitespace(read_pointer * rp) {
  while(is_whitespace(*(rp->loc))) {
    if(*(rp->loc) == '\n') {
      rp->line++;
      rp->col = 1;
    } else {
      rp->col++;
    }
    rp->loc++;
  }
}

pointer read_pair(read_pointer * rp) {
  /* placeholder */
  return new_nil();
}

pointer read_symbol(read_pointer * rp) {
  int length = 0;
  const char * start = rp->loc;
  const char * next = start;
  while(is_alphanumeric(*next)) {
    length++;
    next++;
  }
  char * name = (char *)GC_MALLOC(length + 1);
  strncpy(name, start, length);
  name[length] = 0;
  rp->loc = next;
  rp->col += length;
  return new_symbol(name);
}

pointer read_number(read_pointer * rp) {
  int length = 0;
  const char * start = rp->loc;
  const char * next = start;
  while(is_numeric(*next)) {
    length++;
    next++;
  }
  char * name = (char *)GC_MALLOC(length + 1);
  strncpy(name, start, length);
  name[length] = 0;
  rp->loc = next;
  rp->col += length;
  int64 num;
  sscanf(name, "%lld", &num);
  return new_int(num);
}

pointer read_next(read_pointer * rp) {
  skip_whitespace(rp);
  char next = *(rp->loc);
  if(next == 0) {
    return NULL;
  } else if(next == '(') {
    return read_pair(rp);
  } else if(is_alpha(next)) {
    return read_symbol(rp);
  } else if(is_numeric(next)) {
    return read_number(rp);
  }
  return NULL;
}

pointer read_from_string(const char * input) {
  read_pointer r = {.loc = input, .line = 1, .col = 1};
  read_pointer * rp = &r;
  pointer last_pair = new_nil();
  pointer next_item;
  /* build a reverse list of elements read */
  while(next_item = read_next(rp)) {
    last_pair = new_pair(next_item, last_pair);
  }
  /* reverse it */
  pointer list = reverse(last_pair);
  return list;
}

void print_thing(pointer p) {
  pointer n;
  if(is_pair(p)) {
    printf("(");
    n = p;
    do {
      print_thing(car(n));
      n = cdr(n);
      if(is_pair(n)) {
        printf(" ");
      }
    } while (is_pair(n));
    printf(")");
  } else if(is_symbol(p)) {
    printf("%s", get_symbol(p)->name);
  } else if(is_int(p)) {
    printf("%lld", get_int(p));
  } else if(is_nil(p)) {
    printf("()");
  }
}

/* the rest */

void what_is_it(pointer p) {
  if(is_pair(p)) {
    printf("It's a pair!\n");
  }
  if(is_symbol(p)) {
    printf("It's a symbol!\n");
  }
  if(is_int(p)) {
    printf("It's an int!\n");
  }
}

int main() {
  printf("Hello, world\n");
  pointer p = new_nil();
  what_is_it(p);
  pointer s = new_symbol("foo");
  what_is_it(s);
  pointer i = new_int(100);
  what_is_it(i);
  assert(100 == get_int(i));
  pointer thing1 = new_nil();
  assert(0 == count(thing1));
  pointer thing2 = new_pair(new_int(100), new_nil());
  assert(1 == count(thing2));
  pointer thing3 = new_pair(new_int(100), new_pair(new_symbol("hello"), new_nil()));
  assert(2 == count(thing3));
  pointer thing4 = new_pair(new_int(100), new_nil());
  assert(1 == count(thing4));
  pointer thing5 = new_pair(new_int(100), new_pair(new_nil(), new_nil()));
  assert(2 == count(thing5));
  print_thing(thing1);
  print_thing(thing2);
  print_thing(thing3);
  print_thing(thing4);
  print_thing(thing5);
  printf("\n");

  const char * test_string = "this that 100";
  pointer test = read_from_string(test_string);
  print_thing(test);
  printf("\n");

  const char * test_empty = " ";
  pointer test_nil = read_from_string(test_empty);
  print_thing(test_nil);
  printf("\n");

  return 0;
}

