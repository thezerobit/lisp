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

* built-in functions
  * print and println
  * integer math: + - * /

* rudimentary REPL

TODO
----

* single 'quote syntax sugar
* numerical and string comparison, gt, lt, etc...
* lambdas (fn? lambda?)
* vector goodies
  * vector->list
  * list->vector
* map, set
  * language support
  * literal syntax
* macros
* local bindings (LET)
* top level bindings (DEF, DEFUN)
* keywords
* compilation

