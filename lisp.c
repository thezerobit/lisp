/* BOEHM! */
#include "gc.h"
#include <stdio.h>
#include <string.h>

const TYPE_MASK   = 0b111;

const TYPE_PAIR   = 0b000;
const TYPE_SYMBOL = 0b001;
const TYPE_INT    = 0b010;

typedef void * pointer;
typedef unsigned long long uint64;

/* Pair */

struct pair {
  pointer car;
  pointer cdr;
};

typedef struct pair * Pair;

int is_pair(pointer p) {
  return (TYPE_MASK & (uint64)p) == TYPE_PAIR;
}

Pair get_pair(pointer p) {
  return (Pair)p;
}

pointer new_pair(pointer car, pointer cdr) {
  pointer p = GC_MALLOC(sizeof(struct pair));
  Pair cell = get_pair(p);
  cell->car = car;
  cell->cdr = cdr;
  return p;
}

/* Symbol */

struct symbol {
  char * name;
};

typedef struct symbol * Symbol;

int is_symbol(pointer p) {
  return (TYPE_MASK & (uint64)p) == TYPE_SYMBOL;
}

pointer new_symbol(const char * name) {
  Symbol sym = (Symbol)GC_MALLOC(sizeof(struct symbol));
  sym->name = GC_MALLOC(strlen(name) + 1);
  strcpy(sym->name, name);
  return (pointer)((uint64)sym | TYPE_SYMBOL);
}

Symbol get_symbol(pointer p) {
  return (Symbol)((uint64)p ^ TYPE_MASK);
}

/* Int */

typedef uint64 * Int;

int is_int(pointer p) {
  return (TYPE_MASK & (uint64)p) == TYPE_INT;
}

pointer new_int(uint64 num) {
  Int i = (Int)GC_MALLOC(sizeof(uint64));
  *i = num;
  return (pointer)((uint64)i | TYPE_INT);
}

Int get_int(pointer p) {
  return (Int)((uint64)p ^ TYPE_MASK);
}

/* read */

const char * skip_whitespace(const char * input) {
  while(*input == ' ') {
    input++;
  }
  return input;
}

int is_alpha(char c) {
  int ic = (int)c;
  return (ic >= 65 && ic <= 90) || (ic >= 97 && ic <= 122);
}

int is_numeric(char c) {
  int ic = (int)c;
  return (ic >= 48 && ic <= 57);
}

/* pointer read_from_string(const char * input) { */
/*   input = skip_whitespace(input); */
/*   char next = *input; */
/*   if(next == '(') { */
/*     return read_pair(input); */
/*   } else if(is_alpha(next)) { */
/*     return read_symbol(input); */
/*   } */
/* } */

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
  pointer p = new_pair(NULL, NULL);
  what_is_it(p);
  pointer s = new_symbol("foo");
  what_is_it(s);
  pointer i = new_int(100);
  what_is_it(i);
  printf("The letter 'a': %d, 'z': %d, 'A': %d, 'Z': %d, '0': %d, '1': %d, '9': %d\n",
      (int)'a', (int)'z', (int)'A', (int)'Z', (int)'0', (int)'1', (int)'9');
}

