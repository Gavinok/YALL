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
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex[0].value())));
  expression expr = add(std::vector<expression>{sexpr(1), sexpr(2)});
  CHECK(get<int>(expr.value()) == 3);
}

TEST_CASE("Environment Lookup with arg lookup") {
  environment ev;
  subexprs ex = get<subexprs>(read_string("(+ 1 2)").value());
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex[0].value())));
  expression expr = add(std::vector(ex.begin()+1, ex.end()));
  CHECK(get<int>(expr.value()) == 3);
}

TEST_CASE("Environment Lookup with error handling") {
  environment ev;
  subexprs ex = get<subexprs>(read_string("(+ hello 2)").value());
  auto add = std::get<expression::lisp_function>(ev.get(get<symbol>(ex[0].value())));
  CHECK_THROWS(add(std::vector(ex.begin()+1, ex.end())));
}

TEST_CASE("basic eval with list"){
  environment ev;
  auto e = read_string("(+ 1 2)");
  CHECK(std::get<int>(tru_eval(e, ev)) == 3);
}

TEST_CASE("basic eval with subtract"){
  environment ev;
  auto e = read_string("(- 1 2)");
  CHECK(std::get<int>(tru_eval(e, ev)) == -1);
}

TEST_CASE("basic eval with multiply"){
  environment ev;
  auto e = read_string("(* 3 2)");
  CHECK(std::get<int>(tru_eval(e, ev)) == 6);
}

TEST_CASE("basic eval with equality true"){
  environment ev;
  auto e = read_string("(= 2 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
}

TEST_CASE("basic eval with equality false"){
  environment ev;
  auto e = read_string("(= 3 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
}

// TEST_CASE("basic eval with eq false on expressions"){
//   environment ev;
//   auto e = read_string("(eq (quote (2)) (quote (2)))");
//   CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
// }

TEST_CASE("basic eval with eq false"){
  environment ev;
  auto e = read_string("(eq 3 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
}

// TODO
TEST_CASE("basic eval with eq true symbol"){
  environment ev;
  auto e = read_string("(eq (quote a) (quote a))");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
}

TEST_CASE("basic eval with eq false symbol"){
  environment ev;
  auto e = read_string("(eq (quote a) (quote b))");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == false);
}

TEST_CASE("basic eval with eq true"){
  environment ev;
  auto e = read_string("(eq 2 2)");
  CHECK(std::get<boolean>(tru_eval(e, ev)).value == true);
}

TEST_CASE("basic eval with nested lists"){
  environment ev;
  auto e = read_string("(+ 9 ( - 5 1))");
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
  auto e = read_string("(define x 1)");
  sexpr s = tru_eval(e, ev);
  e = read_string("x");
  CHECK(std::get<int>(tru_eval(e, ev)) == 1);
}

TEST_CASE("new values in the environment with let"){
  environment ev;
  auto e = read_string("(let ((x 1)\n(y 32))\n (+ x y))");
  sexpr s = tru_eval(e, ev);
  e = read_string("x");
  CHECK_THROWS(std::get<int>(tru_eval(e, ev)) == 1);
  CHECK(std::get<int>(s) == 33);
}


TEST_CASE("missing bunding in let block"){
  environment ev;
  auto e = read_string("(let ((x 1)\n(y 32))\n q)");
  CHECK_THROWS(tru_eval(e, ev));
}

TEST_CASE("Access outer block from let block"){
  environment ev;
  auto e = read_string("(define x 30)");
  sexpr s = tru_eval(e, ev);
  e = read_string("(let ((y 32))\n (+ x y))");
  auto tmp = std::get<int>(tru_eval(e, ev));
  CHECK(tmp == 62);
}

TEST_CASE("Shadowing with let blocks"){
  environment ev;
  auto e = read_string("(define x 30)");
  sexpr s = tru_eval(e, ev);
  e = read_string("(let ((x 32)(y 32))\n (+ x y))");
  auto tmp = std::get<int>(tru_eval(e, ev));
  CHECK(tmp == 64);
}

TEST_CASE("Nested let blocks"){
  environment ev;
  auto e = read_string("(let ((x 32)(y 32))\n(let ((y 5))\n(+ x y)))");
  auto tmp = std::get<int>(tru_eval(e, ev));
  CHECK(tmp == 37);
}

TEST_CASE("Check simple 2 arg if statments"){
  environment ev;
  auto e = read_string("(if (= 1 1)\n (+ 2 2))");
  auto tmp = std::get<int>(tru_eval(e, ev));
  CHECK(tmp == 4);
}

TEST_CASE("Check simple 3 arg if statments true"){
  environment ev;
  auto e = read_string("(if (= 1 1)\n (+ 2 2)\n (+ 3 2))");
  auto tmp = std::get<int>(tru_eval(e, ev));
  CHECK(tmp == 4);
}

TEST_CASE("Check simple 3 arg if statments false"){
  environment ev;
  auto e = read_string(R"((if (= 1 2)
                       ; then
                       (+ 2 2)
                       ; else
                       (+ 3 2)))");
  auto tmp = std::get<int>(tru_eval(e, ev));
  CHECK(tmp == 5);
}
