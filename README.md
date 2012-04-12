trivial attempt at writing a lisp

Inspiration
-----------

There's probably nothing in this Lisp that isn't in one of the
following amazing Lisps:

* Common Lisp
* Scheme
* Clojure

Things That Are Done
--------------------
* read
  * symbols
  * lists
  * signed integers
  * strings
  * vectors and vector literals
  * boolean values: true and false

* interpreted evaluation (evaluate)
  * lookup symbols
  * call built-in functions
  * QUOTE
  * IF (false and empty list are false)
  * numerical comparison, gt, lt, gte, lte
  * = and != comparison for all types
  * lambda
  * def : mutable top-level scope
  * let, let\* and letrec
  * TCO (tail call optimization) for lambdas

* built-in functions
  * print and println
  * integer math: + - \* /
  * list: list, first/car, rest/cdr
  * vector: vector, vector->list, list->vector

* rudimentary REPL

TODO
----

* comments
* NIL, SYMBOL\_\*, etc should be constant
  * replace get\_other(p)->type with get\_type(p)
* split stuff into separate files...
* single 'quote syntax sugar
* begin / do / progn
* map, set
  * language support
  * literal syntax
* macros
* defn / defun
* keywords
* compilation

