#include "ra/read.hpp"
#define CATCH_CONFIG_MAIN
#include <iostream>
#include <vector>
#include <catch2/catch.hpp>
#include <functional>
#include <string>
#include <memory>
#include "ra/eval.hpp"

using std::get;
using subexprs = expression::subexprs;
using sexpr = expression::sexpr;

TEST_CASE("Environment Lookup") {
  subexprs ex = get<subexprs>(read_string("(+ 1 2)").value());
  environment ev;
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex.begin()->value())));
  auto tmp = subexprs{sexpr(1), sexpr(2)};
  expression expr = add(tmp.begin(), tmp.size());
  CHECK(get<int>(expr.value()) == 3);
}


TEST_CASE("Environment Lookup with arg lookup") {
  environment ev;
  subexprs ex = get<subexprs>(read_string("(+ 1 2)").value());
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex.begin()->value())));
  expression expr = add((++ex.begin()), ex.size() - 1);
  CHECK(get<int>(expr.value()) == 3);
}
TEST_CASE("Environment Lookup with error handling") {
  environment ev;
  subexprs ex = get<subexprs>(read_string("(+ hello 2)").value());
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex.begin()->value())));
  CHECK_THROWS(add((++ex.begin()), ex.size() - 1));
}
TEST_CASE("addition expression"){
  SECTION("Check add quoted numbers in list"){
    environment env;
    auto e = read_string(R"((+ (car (list -1 )) (car (quote (2)))))");
    CHECK_THROWS(pr_str(tru_eval(e, env)) == "1");
  }

  SECTION("basic eval with list add"){
    environment ev;
    auto e = read_string("(+ 1 2)");
    CHECK(std::get<int>(tru_eval(e, ev)) == 3);
  }

  SECTION("basic eval with list add on empty"){
    environment ev;
    auto e = read_string("(+)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }

  SECTION("basic eval with list add on one arg"){
    environment ev;
    auto e = read_string("(+ 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }

  SECTION("basic eval add with wrong type "){
    environment ev;
    auto e = read_string("(+ (quote ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }

  SECTION("basic eval add with wrong type list "){
    environment ev;
    auto e = read_string("(+ (list ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }

  SECTION("basic eval add with wrong type symbol "){
    environment ev;
    auto e = read_string("(+ (quote a) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
}

TEST_CASE("Check Subtraction"){
  SECTION("simple subtraction"){
    environment ev;
    auto e = read_string("(- 1 2)");
    CHECK(std::get<int>(tru_eval(e, ev)) == -1);
  }
  SECTION("wrong type "){
    environment ev;
    auto e = read_string("(- (quote ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("With wrong type list "){
    environment ev;
    auto e = read_string("(- (list ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Wrong type symbol "){
    environment ev;
    auto e = read_string("(- (quote a) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Basic eval with list sub on empty"){
    environment ev;
    auto e = read_string("(-)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Basic eval with list add on one arg"){
    environment ev;
    auto e = read_string("(- 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
}
TEST_CASE("Check Multiplication"){
  SECTION("simple multiplication"){
    environment ev;
    auto e = read_string("(* 3 2)");
    CHECK(std::get<int>(tru_eval(e, ev)) == 6);
  }
  SECTION("wrong type "){
    environment ev;
    auto e = read_string("(* (quote ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("With wrong type list "){
    environment ev;
    auto e = read_string("(* (list ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Wrong type symbol "){
    environment ev;
    auto e = read_string("(* (quote a) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Basic eval with list sub on empty"){
    environment ev;
    auto e = read_string("(*)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Basic eval with list add on one arg"){
    environment ev;
    auto e = read_string("(* 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
}
TEST_CASE("Check equality"){
  environment ev;
  SECTION("basic eval with equality true"){
    expression e = read_string("(= 2 2)");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
  }
  SECTION("basic eval with equality false"){
    expression e = read_string("(= 3 2)");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
  }
  SECTION("wrong type "){
     expression e = read_string("(= (quote ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("wrong type 2"){
    expression e = read_string("(= (quote ()) (quote ()))");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("wrong type sym"){
    expression e = read_string("(= (quote a) (quote a))");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("With wrong type list "){
    expression e = read_string("(= (list ()) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Wrong type symbol "){
    expression e = read_string("(= (quote a) 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Basic eval with list sub on empty"){
    expression e = read_string("(=)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("Basic eval with list add on one arg"){
    expression e = read_string("(= 1)");
    CHECK_THROWS(std::get<int>(tru_eval(e, ev)));
  }
  SECTION("basic eval with equality on quoted numbers true"){
    auto e = read_string("(= (quote 2) (quote 2))");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
  }
}
// TODO
TEST_CASE("basic eval with eq false on expressions"){
  environment ev;
  auto e = read_string("(eq (quote (2)) (quote (2)))");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
}
TEST_CASE("basic eval with multiplication"){
  environment ev;
  SECTION("Quote no args"){
    auto e = read_string("(quote)");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }

  SECTION("Quote 2 args"){
    auto e = read_string("(quote 1 2)");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }

  SECTION("nested quoting"){
    auto e = read_string("(quote (quote (1 2)))");
    CHECK(pr_str(tru_eval(e, ev)) == "(quote (1 2))");
  }
}

TEST_CASE("Check Lamba"){
  SECTION("lambda no lambda list "){
    environment ev;
    auto e = read_string("(lambda  (+ 1 2))");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }
  SECTION("Check basic lambda example args and uses it"){
    environment ev;
    auto e = read_string("(lambda (x) x)");
    auto tmp = std::get<expression::lisp_function>(tru_eval(e, ev));
    environment::subexprs a{expression(2)};
    CHECK(pr_str(tmp(a.begin(), a.size()).value()) == "2");
  }

  SECTION("Check basic lambda example args and uses it in expression"){
    environment ev;
    auto e = read_string("(lambda (x) (+ 1 x))");
    auto tmp = std::get<expression::lisp_function>(tru_eval(e, ev));
    environment::subexprs a{expression(2)};
    CHECK(pr_str(tmp(a.begin(), a.size()).value()) == "3");
  }

  SECTION("Check lambda in a let"){
    environment ev;
    auto e = read_string("(let ((q 20)) (lambda (x) (+ q x)))");
    auto tmp = std::get<expression::lisp_function>(tru_eval(e, ev));
    environment::subexprs a{expression(1)};
    CHECK(pr_str(tmp(a.begin(), a.size()).value()) == "21");
  }

  SECTION("basic eval with eq in lambda"){
    environment ev;
    auto e = read_string("((lambda (x) (eq x (quote a))) (car (quote (a b))))");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
  }
  SECTION("Check nested quoting in lambda"){
    auto e = read_string("(define x (lambda () (if (= 1 1) (quote a))))");
    environment env;
    pr_str(tru_eval(e, env));
    e = read_string("(x)");
    CHECK(pr_str(tru_eval(e, env)) == "a");
  }

  SECTION("Check nested quoting in lambda double def"){
    environment env;
    auto e = read_string("(define not (lambda (condition) (if condition #f #t)))");
    pr_str(tru_eval(e, env));
    e = read_string("(define unless (lambda (condition expression) (if (not condition) expression)))");
    pr_str(tru_eval(e, env));
    e = read_string("(unless #f (quote (print this)))");
    CHECK(pr_str(tru_eval(e, env)) == "(print this)");
  }
  SECTION("Check let inside definition with lambda"){
    environment env;
    auto e = read_string(R"((define doubleAndInc
                             (let ((double (lambda (x) (* 2 x))))
                                    (lambda (q)
                                            (+ 1 (double q))))))");
    tru_eval(e, env);
    e = read_string(R"((doubleAndInc 200))");
    CHECK(pr_str(tru_eval(e, env)) == "401");
  }
  SECTION("Check proposal let with lambda"){
    environment env;
    auto e = read_string(R"((define x (let ((x 1))
                      (lambda (y)
                         (+ x y)))))");
    pr_str(tru_eval(e, env));
    e = read_string(R"((x 2))");
    CHECK(pr_str(tru_eval(e, env)) == "3");
  }

  SECTION("Check direct call to lambda"){
    environment env;
    auto e = read_string(R"(((lambda ()
                              (+ 1 2))))");
    CHECK(pr_str(tru_eval(e, env)) == "3");
  }
  SECTION("Check direct call to lambda with argument"){
    environment env;
    auto e = read_string(R"(((lambda (x)
                              (+ 1 x)) 2))");
    CHECK(pr_str(tru_eval(e, env)) == "3");
  }
}
TEST_CASE("Check define"){
  SECTION("define no args"){
    environment ev;
    auto e = read_string("(define)");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }

  SECTION("define 3 args"){
    environment ev;
    auto e = read_string("(define x 1 2)");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }

  SECTION("define 1 args"){
    environment ev;
    auto e = read_string("(define x)");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }
}
TEST_CASE("Check Let"){
  SECTION("let no let bindings"){
    environment ev;
    auto e = read_string("(let (+ 1 2))");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }
  SECTION("let empty let bindings"){
    environment ev;
    auto e = read_string("(let () (+ 1 2))");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }
  SECTION("let 3 entries in a let binding"){
    environment ev;
    auto e = read_string("(let ((x 1 2)) (+ 1 2))");
    CHECK_THROWS(std::get<boolean>(tru_eval(e, ev)).value);
  }
  SECTION("missing bunding in let block"){
    environment ev;
    auto e = read_string("(let ((x 1)\n(y 32))\n q)");
    CHECK_THROWS(tru_eval(e, ev));
  }

  SECTION("Access outer block from let block"){
    environment ev;
    auto e = read_string("(define x 30)");
    sexpr s = tru_eval(e, ev);
    e = read_string("(let ((y 32))\n (+ x y))");
    auto tmp = std::get<int>(tru_eval(e, ev));
    CHECK(tmp == 62);
  }

  SECTION("Shadowing with let blocks"){
    environment ev;
    auto e = read_string("(define x 30)");
    sexpr s = tru_eval(e, ev);
    e = read_string("(let ((x 32)(y 32))\n (+ x y))");
    auto tmp = std::get<int>(tru_eval(e, ev));
    CHECK(tmp == 64);
  }
  SECTION("Nested let blocks"){
    environment ev;
    auto e = read_string("(let ((x 32)(y 32))\n(let ((y 5))\n(+ x y)))");
    auto tmp = std::get<int>(tru_eval(e, ev));
    CHECK(tmp == 37);
  }
}
TEST_CASE("Check eq"){
  SECTION("basic eval with eq false"){
    environment ev;
    auto e = read_string("(eq 3 2)");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
  }
  // TODO
  SECTION("basic eval with eq true symbol"){
    environment ev;
    auto e = read_string("(eq (quote a) (quote a))");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
  }
  SECTION("basic eval with eq false symbol"){
    environment ev;
    auto e = read_string("(eq (quote a) (quote b))");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
  }
  SECTION("basic eval with eq true"){
    environment ev;
    auto e = read_string("(eq 2 2)");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
  }
  SECTION("basic eval with eq expr that evals to quoted expression true symbol"){
    environment ev;
    auto e = read_string("(define b  (quote a))");
    tru_eval(e, ev);
    e = read_string("(eq b (quote a))");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
  }
  SECTION("basic eval with eq expr that evals to quoted expression true symbol false"){
    environment ev;
    auto e = read_string("(define b  (quote b))");
    tru_eval(e, ev);
    e = read_string("(eq b (quote a))");
    CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
  }

  SECTION("Check eq on symbol"){
    environment env;
    auto e = read_string(R"((eq (car (quote (a 2 3))) (quote a)))");
    CHECK(pr_str(tru_eval(e, env)) == "#t");
  }

  SECTION("Check eq on car num"){
    environment env;
    auto e = read_string(R"((eq (car (quote (1 2 3))) 1))");
    CHECK(pr_str(tru_eval(e, env)) == "#t");
  }
}
TEST_CASE("basic eval with nested lists"){
  environment ev;
  auto e = read_string("(+ 9 ( - 5 1))");
  CHECK(std::get<int>(tru_eval(e, ev)) == 13);
}
TEST_CASE("new values in the environment with tru_eval"){
  environment ev;
  auto e = read_string("(define x 1)");
  sexpr s = tru_eval(e, ev);
  e = read_string("x");
  CHECK(std::get<int>(tru_eval(e, ev)) == 1);
}
TEST_CASE("Check if"){
    environment ev;
  SECTION("Check simple 2 arg if statments"){
    auto e = read_string("(if (= 1 1)\n (+ 2 2))");
    auto tmp = std::get<int>(tru_eval(e, ev));
    CHECK(tmp == 4);
  }
  SECTION("Check simple 3 arg if statments true"){
    auto e = read_string("(if (= 1 1)\n (+ 2 2)\n (+ 3 2))");
    auto tmp = std::get<int>(tru_eval(e, ev));
    CHECK(tmp == 4);
  }
  SECTION("Check simple 3 arg if statments false"){
    auto e = read_string(R"((if (= 1 2)
                       ; then
                       (+ 2 2)
                       ; else
                       (+ 3 2)))");
    auto tmp = std::get<int>(tru_eval(e, ev));
    CHECK(tmp == 5);
  }
}
TEST_CASE("Check nested quoting"){
  auto e = read_string("(define x (if (= 1 1) (quote a)))");
  environment env;
  pr_str(tru_eval(e, env));
  e = read_string("x");
  CHECK(pr_str(tru_eval(e, env)) == "a");
}
TEST_CASE("Check proposal nested let"){
  environment env;
  auto e = read_string(R"((let ((x 1)
                         (y 9))
                      (let ((x 5))
                         (+ x y))))");
  CHECK(pr_str(tru_eval(e, env)) == "14");
}

TEST_CASE("Check nested quoting in lambda double def"){
  environment env;
  auto e = read_string("(define not (lambda (condition) (if condition #f #t)))");
  pr_str(tru_eval(e, env));
  e = read_string("(define unless (lambda (condition expression) (if (not condition) expression)))");
  pr_str(tru_eval(e, env));
  e = read_string("(unless #f (quote (print this)))");
  CHECK(pr_str(tru_eval(e, env)) == "(print this)");
}

TEST_CASE("Check let inside definition with lambda"){
  environment env;
  auto e = read_string(R"((define doubleAndInc
                             (let ((double (lambda (x) (* 2 x))))
                                    (lambda (q)
                                            (+ 1 (double q))))))");
  tru_eval(e, env);
  e = read_string(R"((doubleAndInc 200))");
  CHECK(pr_str(tru_eval(e, env)) == "401");
}

TEST_CASE("Check proposal nested let"){
  environment env;
  auto e = read_string(R"((let ((x 1)
                         (y 9))
                      (let ((x 5))
                         (+ x y))))");
  CHECK(pr_str(tru_eval(e, env)) == "14");
}

TEST_CASE("Check proposal let with lambda"){
  environment env;
  auto e = read_string(R"((define x (let ((x 1))
                      (lambda (y)
                         (+ x y)))))");
  pr_str(tru_eval(e, env));
   e = read_string(R"((x 2))");
   CHECK(pr_str(tru_eval(e, env)) == "3");
}

TEST_CASE("Check direct call to lambda"){
  environment env;
  auto e = read_string(R"(((lambda ()
                              (+ 1 2))))");
   CHECK(pr_str(tru_eval(e, env)) == "3");
}

TEST_CASE("Check direct call to lambda with argument"){
  environment env;
  auto e = read_string(R"(((lambda (x)
                              (+ 1 x)) 2))");
   CHECK(pr_str(tru_eval(e, env)) == "3");
}

TEST_CASE("Check car"){
  environment env;
  auto e = read_string(R"((car (quote (1 2 3))))");
  CHECK(pr_str(tru_eval(e, env)) == "1");
}

TEST_CASE("Check cdr"){
  environment env;
  auto e = read_string(R"((cdr (quote (1 2 3))))");
  CHECK(pr_str(tru_eval(e, env)) == "(2 3)");
}
