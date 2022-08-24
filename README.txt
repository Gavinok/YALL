1 YALL DOCUMENTATION
====================

1.1 Installation
~~~~~~~~~~~~~~~~

1.1.1 Dependencies
------------------

  To install YALL you will need `cmake' installed.


1.1.2 Install Process
---------------------

  To build the project and install it in you ~/.local/bin/

  Run the command

  ,----
  | cmake -H. -Btmp -DCMAKE_INSTALL_PREFIX=~/.local/bin/
  `----

  Then

  ,----
  | cmake --build tmp --clean-first --target install
  `----


1.2 YALL a simple extensible lisp dialect
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  This program consists of two parts 1 the demo script which feeds some
  example code into a non interactive YALL process.

  The other part is an the yall executable it's self.

  you can run it interactively with

  ,----
  | lisp -i
  `----


  or

  ,----
  | lisp --interactive
  `----

  If you want to pass it input from an existing file run

  ,----
  | lisp < file_to_run.scm
  `----

  Since yall is heavily inspired by scheme it's recommended to set your
  editor to its' scheme or at least lisp mode for better syntax
  highlighting.


1.3 Functions Available In YALL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1.3.1 +
-------

  adds 2 values together and returns the result as an integer
  ,----
  | (+ <symboli-expression> <symboli-expression>)
  `----


1.3.2 -
-------

  subtract the first argument from the second argument and returns the
  result as an integer
  ,----
  | (- <symboli-expression> <symboli-expression>)
  `----


1.3.3 *
-------

  multiply 2 values together and returns the result as an integer
  ,----
  | (* <symboli-expression> <symboli-expression>)
  `----


1.3.4 =
-------

  Compares 2 integers for equivalence and returns the result as a
  boolean
  ,----
  | (= <symboli-expression> <symboli-expression>)
  `----


1.3.5 eq
--------

  Compares 2 expressions for equivalence and returns the result as a
  boolean (note that all functions equivalent)
  ,----
  | (eq <symboli-expression> <symboli-expression>)
  `----


1.3.6 if
--------

  Takes 2 or 3 arguments. the first being a condition that results in a
  boolean the second being the expression to return if this is true and
  the optional third will be returned if it is false.

  If no third argument is provided and the condition is evaluated to be
  #f then the #f is returned.

  ,----
  | (if <condition> <then> <else>)
  `----


1.3.7 list
----------

  Takes a variable number of arguments of at least one and returns the
  result as a list.
  ,----
  | (list <symboli-expression>+)
  `----


1.3.8 append
------------

  Takes 2 lists and appends them then returns the result as a list.
  ,----
  | (append <list> <list>)
  `----


1.3.9 cons
----------

  Takes 2 arguments and produces either a cons cell or a list.

  If the second argument is a list then it will return a list with the
  first argument at the front of the list

  Otherwise a cons cell is returned with the first and second arguments
  as it's contents.

  ,----
  | (cons <symbolic-expression> <symbolic-expression>)
  `----


1.3.10 car
----------

  Takes one argument of either a cons cell or a list and returns the
  first element.

  ,----
  | (car <cons-or-list>)
  `----


1.3.11 cdr
----------

  Takes one argument of either a cons cell or a list and returns
  everything other than the first argument.
  ,----
  | (cdr <cons-or-list>)
  `----


1.3.12 sleep
------------

  Takes one argument of an integer n and halts the YALL for n seconds
  the number of seconds is then returned.
  ,----
  | (sleep <int>)
  `----


1.3.13 print
------------

  Takes a symbolic expression and prints the string representation. It's
  argument is then used as it's returned value.
  ,----
  | (print <expression>)
  `----


1.4 Supported Types
~~~~~~~~~~~~~~~~~~~

1.4.1 Supported Symbols
-----------------------

  Symbols can contain a series of letters, numbers and dashes however
  the first character must always be a letter.

  The exception to this is the built in symbols *, -, and +

  The regular expression for this would look something like
  [a-Z]([a-Z]|-|[0-9])+


1.4.2 Numbers
-------------

  Numbers can be any valid C++ int. There is no attempt by YALL to
  handle integer overflow.


1.4.3 Booleans
--------------

  The only supported booleans are #t for true and #f for false


1.4.4 Containers
----------------

* 1.4.4.1 Lists

  Lists in YALL can be created with the list function

  (list + 2 3) => (#<YALL Function> 2 3)

  Alternatively quoting an expression will also create a list without
  evaluating it's contents

  (quote (+ 2 3)) => (+ 2 3)


* 1.4.4.2 Cons Cells

  Lists in YALL can be created with the cons function

  (cons 1 2) => (1 . 2)

  Alternatively quoting a dotted expression to create a cons cell
  without evaluating it's contents

  (quote (+ . 2)) => (+ . 2)
1 Language Overview
===================

1.1 What is YALL?
~~~~~~~~~~~~~~~~~

  YALL is a lisp dialect inspired by Scheme


1.2 How does it work
~~~~~~~~~~~~~~~~~~~~

  YALL can be ran interactively and non interactively

  to run it interactively call it with the `-i'

  We can now begin evaluating expressions


1.3 What are expressions
~~~~~~~~~~~~~~~~~~~~~~~~

  Expressions are a made up of what are known as symbolic expressions.

  an expression consists of to parentheses

  Here is an example

  ,----
  | (+ 1 2)
  `----


  The first argument is a function call and the others are it's
  arguments

  The when ran this evaluates to

  ,----
  | 3
  `----


1.4 Functions
~~~~~~~~~~~~~

  YALL has a single namespace for both functions and variables so you
  can store and call other functions.

  functions are created with the lambda keyword followed by a list of
  arguments and th body of the function
  ,----
  | (lambda (argOne argTwo) ; arguments
  |   (+ argOne argTwo))  ; body
  `----


  We can store values using define

  ,----
  | (define double
  |   (lambda (a) (* a 2)))
  `----

  Then we can call the function just like any built in one

  ,----
  | (double 10)
  `----

  Since functions act as variables you can also pass them as arguments

  This will partially apply the first argument
  ,----
  | (define curry
  |      (lambda (fn arg)
  |        (lambda (y) (fn 1 y))))
  `----

  ,----
  | (curry + 1) ; a function 
  `----

  ,----
  | ((curry + 1) 2)
  `----


1.5 Definitions are limited
~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Unfortunately a current limitation of YALL is that bindings can not be
  back referenced.

  ,----
  | (define caller (lambda (x) (x 1)))
  `----

  ,----
  | (define isOne (lambda (num) (= num 1)))
  `----

  ,----
  | (caller isOne)
  `----

  will work but

  ,----
  | (define caller (lambda () (isOne 1)))
  `----

  ,----
  | (define isOne (lambda (num) (= num 1)))
  `----

  ,----
  | (caller)
  `----

  Will fail

  For this same reason recursion is not possible.


1.6 let
~~~~~~~

  In addition can define new lexically scoped blocks using `let'

  ,----
  | (let ((a 1)
  |       (b 2))
  |   (+ a b))
  `----

  Just like anything else you can also define functions in let
  declarations.


1.7 Code as data
~~~~~~~~~~~~~~~~

  YALL also allows for code to be used as data. Use the quote keyword to
  prevent evaluation of an expression.

  ,----
  | (quote (+ 1 2))
  `----

  Then you can access the contents with the functions `car' and `cdr'

  You can think of them as the same thing as first and rest
  ,----
  | (car (quote (+ 1 2)))
  `----

  Just due to the time constants of the course proper meta programming
  (since you can not unquote an expression) is not fully supported since
  there is now.

  for example
  ,----
  | ((car (quote (+ 1 2))) 1 2)  
  `----

  will fail to function since the `+' will remain quoted.

  However accessing numbers will function as expected

  ,----
  | (+  (car (quote (1 2))) 3)
  `----

  For this reason I included the `list' function but this will allow for
  `+' to evaluate to a function

  ,----
  | ((car (list + 1 2)) 1 2)  
  `----


1.8 YALL supports conditional logic
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  #t is for true f is for false

  The only built in conditional logic is `if'
  ,----
  | (if (= 1 1)
  |     (quote (yes they are))
  |     (quote (no they are not)))
  `----

  With this most common logic can be defined
  ,----
  | (define not (lambda (condition)
  | 	      (if condition
  | 		  #f
  | 		  #t)))
  `----

  ,----
  | (not (= 1 1))
  `----

  you can compare symbols with `eq'

  Note that to compare symbols they need to be quoted

  otherwise their values will be compared

  src scheme (eq (quote a) (quote b)) #+end_src

  src scheme (eq (quote a) (quote a)) #+end_src

  ,----
  | (eq 1 1)
  `----


1.9 Acknowledgments
~~~~~~~~~~~~~~~~~~~

  Since this was my first attempt at writing a proper parser lot's of
  research was necessary to get started.

  I would like to thank the Make a Lisp Project in particular the

  <https://github.com/kanaka/mal/blob/master/process/guide.md#step1>

  Which gave me a good Idea of what was needed to write a proper
  tokenizer and parser.

  No code was used from the project but multiple ideas inspired my
  reader implementation.

  Since my language (scheme like) is quite different from the one
  created in MAL (clojure like) only the core ideas of writing a lisp
  parser where able to be reused.
