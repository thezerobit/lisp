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

* built-in functions
  * print and println
  * integer math: + - * /

* rudimentary REPL

TODO
----

* single 'quote syntax sugar
* begin / do / progn
* vector goodies
  * vector->list
  * list->vector
* map, set
  * language support
  * literal syntax
* macros
* local bindings (LET)
* defn / defun
* keywords
* compilation

