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
  subexprs ex = get<subexprs>(READ("(+ 1 2)").value());
  environment ev;
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex[0].value())));
  expression expr = add(std::vector<expression>{sexpr(1), sexpr(2)});
  CHECK(get<int>(expr.value()) == 3);
}

TEST_CASE("Environment Lookup with arg lookup") {
  environment ev;
  subexprs ex = get<subexprs>(READ("(+ 1 2)").value());
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex[0].value())));
  expression expr = add(std::vector(ex.begin()+1, ex.end()));
  CHECK(get<int>(expr.value()) == 3);
}

TEST_CASE("Environment Lookup with error handling") {
  environment ev;
  subexprs ex = get<subexprs>(READ("(+ hello 2)").value());
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex[0].value())));
  CHECK_THROWS(add(std::vector(ex.begin()+1, ex.end())));
}

TEST_CASE("basic eval with list"){
  environment ev;
  auto e = READ("(+ 1 2)");
  CHECK(std::get<int>(tru_eval(e, ev)) == 3);
}

TEST_CASE("basic eval with subtract"){
  environment ev;
  auto e = READ("(- 1 2)");
  CHECK(std::get<int>(tru_eval(e, ev)) == -1);
}

TEST_CASE("basic eval with multiply"){
  environment ev;
  auto e = READ("(* 3 2)");
  CHECK(std::get<int>(tru_eval(e, ev)) == 6);
}

TEST_CASE("basic eval with equality true"){
  environment ev;
  auto e = READ("(= 2 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
}

TEST_CASE("basic eval with equality false"){
  environment ev;
  auto e = READ("(= 3 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
}

TEST_CASE("basic eval with eq false"){
  environment ev;
  auto e = READ("(eq 3 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
}

// TODO
TEST_CASE("basic eval with eq true symbol"){
  environment ev;
  auto e = READ("(eq (quote a) (quote a))");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
}

TEST_CASE("basic eval with eq false symbol"){
  environment ev;
  auto e = READ("(eq (quote a) (quote b))");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
}

TEST_CASE("basic eval with eq true"){
  environment ev;
  auto e = READ("(eq 2 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
}

TEST_CASE("basic eval with nested lists"){
  environment ev;
  auto e = READ("(+ 9 ( - 5 1))");
  CHECK(std::get<int>(tru_eval(e, ev)) == 13);
}

std::shared_ptr<expression> tester(expression e){
  return std::make_shared<expression>(e);
}

TEST_CASE("Create a shared expression"){
 auto tmp = tester(expression(sexpr(3)));
}

TEST_CASE("new values in the environment with tru_eval"){
  environment ev;
  auto e = READ("(define x 1)");
  sexpr s = tru_eval(e, ev);
  e = READ("x");
  CHECK(std::get<int>(tru_eval(e, ev)) == 1);
}

TEST_CASE("new values in the environment with let"){
  environment ev;
  auto e = READ("(let ((x 1)\n(y 32))\n (+ x y))");
  sexpr s = tru_eval(e, ev);
  e = READ("x");
  CHECK_THROWS(std::get<int>(tru_eval(e, ev)) == 1);
  CHECK(std::get<int>(s) == 33);
}


