#define CATCH_CONFIG_MAIN
#include <iostream>
#include <vector>
#include <catch2/catch.hpp>
#include <functional>
#include <string>
#include "ra/read.hpp"

TEST_CASE("Check tokenizer"){
  auto t = tokenizer("(hello world)");
  CHECK(t[0] == "(");
  CHECK(t[1] == "hello");
  CHECK(t[2] == "world");
  CHECK(t[3] == ")");
}

TEST_CASE("Check tokenizer nested lists"){
  auto t = tokenizer("(hello (hello world))");
  CHECK(t[0] == "(");
  CHECK(t[1] == "hello");
  CHECK(t[2] == "(");
  CHECK(t[3] == "hello");
  CHECK(t[4] == "world");
  CHECK(t[5] == ")");
  CHECK(t[6] == ")");
}

TEST_CASE("Check Reader constructor"){
  Reader r(std::vector<std::string>{"hello", "world"});
  CHECK(r.peak() == "hello");
  CHECK(r.next() == "hello");
  CHECK(r.peak() == "world");
  // TODO determine how we want to handle when there is nothing left to read
}

TEST_CASE("Check Tokenizer to Reader"){
  auto t = tokenizer("(hello (hello world))");
  Reader r(t);
  CHECK(r.peak() == "(");
  CHECK(r.next() == "(");
  CHECK(r.peak() == "hello");
  CHECK(r.next() == "hello");
  CHECK(r.next() == "(");
  CHECK(r.next() == "hello");
  CHECK(r.next() == "world");
  // TODO determine how we want to handle when there is nothing left to read
}

TEST_CASE("Check Tokenizer to Reader actual code"){
  auto t = tokenizer("(- 30 (+ 1 2))");
  Reader r(t);
  CHECK(r.peak() == "(");
  CHECK(r.next() == "(");
  CHECK(r.peak() == "-");
  CHECK(r.next() == "-");
  CHECK(r.next() == "30");
  CHECK(r.next() == "(");
  CHECK(r.next() == "+");
  CHECK(r.next() == "1");
  CHECK(r.next() == "2");
  CHECK(r.next() == ")");
  CHECK(r.next() == ")");
}

TEST_CASE("Check comments"){
  auto t = tokenizer("(hello ) ; dont show this");
  Reader r(t);
  CHECK(r.next() == "(");
  CHECK(r.next() == "hello");
  CHECK(r.next() == ")");
  CHECK(r.next() == ";");
}



TEST_CASE("Check all builtins"){
  auto t = tokenizer("+ - * ; = . () \n #f #t");
  Reader r(t);
  CHECK(r.next() == "+");
  CHECK(r.next() == "-");
  CHECK(r.next() == "*");
  CHECK(r.next() == ";");
  CHECK(r.next() == "=");
  CHECK(r.next() == ".");
  CHECK(r.next() == "(");
  CHECK(r.next() == ")");
  CHECK(r.next() == "\n");
  CHECK(r.next() == "#f");
  CHECK(r.next() == "#t");
}

TEST_CASE("Check read_form with list of numbers"){
  using subexprs = expression::subexprs;
  Reader r(tokenizer("(1 2 3)"));
  try {
    subexprs e = std::get<subexprs>(read_form(r));
    auto expected = std::vector<int>{1, 2, 3};
    subexprs::iterator  x = e.begin();
    std::vector<int>::iterator y = expected.begin();
    for(; x != e.end(); x++, y++){
      CHECK(std::get<int>((*x).value()) == *y);
    }
  } catch (const std::runtime_error& e) {
    std::cout << e.what();
  }
}

TEST_CASE("Check read_form with sub lists", "[constructor]") {
  using subexprs = expression::subexprs;
  Reader r(tokenizer("(1 (2 3) 4)"));
  subexprs e = std::get<subexprs>(read_form(r));

  CHECK(e.size() == 3);
  CHECK(std::get<subexprs>(e.at(1).value()).size() == 2);

  auto expected = std::vector<int>{1, 2, 3, 4};
  auto valiter = expected.begin();

  for(auto i = e.begin(); i != e.end(); i++){
    std::function<void(expression)> checker;
    checker = [&checker, &valiter](expression x) {
      std::visit(overloaded{
          [&checker](subexprs x){
            for (auto v : x){
              checker(v.value());
            }},
           [&valiter](int x){
             CHECK(x == *valiter);
             valiter++;
           },
            [](auto x [[gnu::unused]]){
             FAIL();
           }},
        x.value());
    };
    checker(i->value());
  }
}

TEST_CASE("Check read_form recognize symbols", "[constructor]") {
  using subexprs = expression::subexprs;
  Reader r(tokenizer("(hello (there 3) 4)"));
  subexprs e = std::get<subexprs>(read_form(r));
  auto expected = std::vector<std::variant<int, symbol>>{"hello", "there", 3, 4};
  auto valiter = expected.begin();
  CHECK(e.size() == 3);
  CHECK(std::get<subexprs>(e.at(1).value()).size() == 2);
  for(auto i = e.begin(); i != e.end(); i++){
    std::function<void(expression)> checker;
    checker = [&checker, &valiter](expression x) {
      std::visit(overloaded{
          [&checker](subexprs x){
            for (auto v : x){
              checker(v.value());
            }},
           [&valiter](int x){
             CHECK(x == get<int>(*valiter));
             valiter++;
           },
           [&valiter](symbol x){
             CHECK(x == get<symbol>(*valiter));
             valiter++;
           },
            [](auto x [[gnu::unused]]){
              FAIL();
           }},
        x.value());
    };
    checker(i->value());
  }
}

TEST_CASE("Check read_form recognize booleans", "[constructor]") {
  using subexprs = expression::subexprs;
  Reader r(tokenizer("(hello (#f #f) #t)"));
  subexprs e = std::get<subexprs>(read_form(r));
  auto expected = std::vector<std::variant<int, symbol, boolean>>{"hello", boolean{false}, boolean{false}, boolean{true}};

  CHECK(e.size() == 3);
  CHECK(std::get<subexprs>(e.at(1).value()).size() == 2);

  auto valiter = expected.begin();
  for(auto i = e.begin(); i != e.end(); i++){
    std::function<void(expression)> checker;
    checker = [&checker, &valiter](expression x) {
      std::visit(overloaded{
          [&checker](subexprs x){
            for (auto v : x){
              checker(v.value());
            }},
           [&valiter](int x){
             CHECK(x == get<int>(*valiter));
             valiter++;
           },
           [&valiter](symbol x){
             CHECK(x == get<symbol>(*valiter));
             valiter++;
           },
           [&valiter](boolean x){
             CHECK(x.value == get<boolean>(*valiter).value);
             valiter++;
           },
           [](auto x [[gnu::unused]]){
              FAIL();
           }},
        x.value());
    };
    checker(i->value());
  }
}
TEST_CASE("Check read_form ignore comments", "[constructor]") {
  using subexprs = expression::subexprs;
  Reader r(tokenizer("(1 (2 ;# thi )))\n 3) 4)"));
  subexprs e = std::get<subexprs>(read_form(r));
  std::vector<std::variant<int, symbol, boolean>>expected{1, 2, 3, 4};
  CHECK(e.size() == 3);
  CHECK(std::get<subexprs>(e.at(1).value()).size() == 2);
  auto valiter = expected.begin();
  for(auto i = e.begin(); i != e.end(); i++){
    std::function<void(expression)> checker;
    checker = [&checker, &valiter](expression x) {
      std::visit(overloaded{
          [&checker](subexprs x){
            for (auto v : x){
              checker(v.value());
            }},
           [&valiter](int x){
             CHECK(x == get<int>(*valiter));
             valiter++;
           },
            [](auto x [[gnu::unused]]){
              FAIL();
           }},
        x.value());
    };
    checker(i->value());
  }
}
TEST_CASE("Check read_form handle built in symbols", "[constructor]") {
  using subexprs = expression::subexprs;
  auto t = tokenizer("(- * = (+ 3) 4)");
  Reader r(t);
  subexprs e = std::get<subexprs>(read_form(r));
  std::vector<std::variant<int, symbol, boolean>> expected{"-", "*", "=", "+", 3, 4};
  CHECK(e.size() == 5);
  CHECK(std::get<subexprs>(e.at(3).value()).size() == 2);
  auto valiter = expected.begin();
  for(auto i = e.begin(); i != e.end(); i++){
    std::function<void(expression)> checker;
    checker = [&checker, &valiter](expression x) {
      std::visit(overloaded{
          [&checker](subexprs x){
            for (auto v : x){
              checker(v.value());
            }},
           [&valiter](int x){
             CHECK(x == get<int>(*valiter));
             valiter++;
           },
           [&valiter](symbol x){
             CHECK(x == get<symbol>(*valiter));
             valiter++;
           },
            [](auto x [[gnu::unused]]){
              FAIL();
           }},
        x.value());
    };
    checker(i->value());
  }
}
TEST_CASE("Check pr_str with sub lists", "[constructor]") {
  auto exp = "(1 (2 (1 9 3)) 4)";
  auto t = tokenizer(exp);
  Reader r(t);
  CHECK(pr_str(read_form(r)) == exp);
}

TEST_CASE("Check unclosed expression", "[constructor]") {
  auto exp = "(1 (2 (1 9 3) 4)";
  auto t = tokenizer(exp);
  Reader r(t);
  CHECK_THROWS(read_form(r));
}
