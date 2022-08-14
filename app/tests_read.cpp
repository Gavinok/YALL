#define CATCH_CONFIG_MAIN
#include "ra/read.hpp"
#include <catch2/catch.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using subexprs = expression::subexprs;

TEST_CASE("Check tokenizer") {
  SECTION("single list") {
    auto t = tokenizer("(hello world)");
    CHECK(t[0] == "(");
    CHECK(t[1] == "hello");
    CHECK(t[2] == "world");
    CHECK(t[3] == ")");
  }

  SECTION("Check tokenizer nested lists") {
    auto t = tokenizer("(hello (hello world))");
    CHECK(t[0] == "(");
    CHECK(t[1] == "hello");
    CHECK(t[2] == "(");
    CHECK(t[3] == "hello");
    CHECK(t[4] == "world");
    CHECK(t[5] == ")");
    CHECK(t[6] == ")");
  }

  SECTION("Check Tokenizer to Reader") {
    auto t = tokenizer("(hello (hello world))");
    Reader r(t);
    CHECK(r.peak() == "(");
    CHECK(r.next() == "(");
    CHECK(r.peak() == "hello");
    CHECK(r.next() == "hello");
    CHECK(r.next() == "(");
    CHECK(r.next() == "hello");
    CHECK(r.next() == "world");
  }
}
TEST_CASE("Check Reader") {
  SECTION("Check Reader constructor") {
    Reader r(std::vector<std::string>{"hello", "world"});
    CHECK(r.peak() == "hello");
    CHECK(r.next() == "hello");
    CHECK(r.peak() == "world");
    // TODO determine how we want to handle when there is nothing left to read
  }

  SECTION("Check Tokenizer to Reader actual code") {
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

  SECTION("Check comments") {
    auto t = tokenizer("(hello ) ; dont show this");
    Reader r(t);
    CHECK(r.next() == "(");
    CHECK(r.next() == "hello");
    CHECK(r.next() == ")");
    CHECK(r.next() == ";");
  }

  SECTION("Check all builtins") {
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
}

TEST_CASE("Check read_form") {
  SECTION("Check read_form with list of numbers") {
    Reader r(tokenizer("(1 2 3)"));
    try {
      subexprs e = std::get<subexprs>(read_form(r));
      auto expected = std::vector<int>{1, 2, 3};
      subexprs::iterator x = e.begin();
      std::vector<int>::iterator y = expected.begin();
      for (; x != e.end(); x++, y++) {
        CHECK(std::get<int>((*x).value()) == *y);
      }
    } catch (const std::runtime_error &e) {
      std::cout << e.what();
    }
  }

  SECTION("sub lists", "[constructor]") {
    Reader r(tokenizer("(1 (2 3) 4)"));
    subexprs e = std::get<subexprs>(read_form(r));

    CHECK(e.size() == 3);
    CHECK(std::get<subexprs>((++e.begin())->value()).size() == 2);

    auto expected = std::vector<int>{1, 2, 3, 4};
    auto valiter = expected.begin();

    for (auto i = e.begin(); i != e.end(); i++) {
      std::function<void(expression)> checker;
      checker = [&checker, &valiter](expression x) {
        std::visit(overloaded{[&checker](subexprs x) {
                                for (auto v : x) {
                                  checker(v.value());
                                }
                              },
                              [&valiter](int x) {
                                CHECK(x == *valiter);
                                valiter++;
                              },
                              [](auto x [[gnu::unused]]) { FAIL(); }},
                   x.value());
      };
      checker(i->value());
    }
  }

  SECTION("recognize symbols", "[constructor]") {
    Reader r(tokenizer("(hello (there 3) 4)"));
    subexprs e = std::get<subexprs>(read_form(r));
    auto expected =
        std::vector<std::variant<int, symbol>>{"hello", "there", 3, 4};
    auto valiter = expected.begin();
    CHECK(e.size() == 3);
    CHECK(std::get<subexprs>((++e.begin())->value()).size() == 2);
    for (auto i = e.begin(); i != e.end(); i++) {
      std::function<void(expression)> checker;
      checker = [&checker, &valiter](expression x) {
        std::visit(overloaded{[&checker](subexprs x) {
                                for (auto v : x) {
                                  checker(v.value());
                                }
                              },
                              [&valiter](int x) {
                                CHECK(x == get<int>(*valiter));
                                valiter++;
                              },
                              [&valiter](symbol x) {
                                CHECK(x == get<symbol>(*valiter));
                                valiter++;
                              },
                              [](auto x [[gnu::unused]]) { FAIL(); }},
                   x.value());
      };
      checker(i->value());
    }
  }

  SECTION("recognize booleans", "[constructor]") {

    Reader r(tokenizer("(hello (#f #f) #t)"));
    subexprs e = std::get<subexprs>(read_form(r));
    auto expected = std::vector<std::variant<int, symbol, boolean>>{
        "hello", boolean{false}, boolean{false}, boolean{true}};

    CHECK(e.size() == 3);
    CHECK(std::get<subexprs>((++e.begin())->value()).size() == 2);

    auto valiter = expected.begin();
    for (auto i = e.begin(); i != e.end(); i++) {
      std::function<void(expression)> checker;
      checker = [&checker, &valiter](expression x) {
        std::visit(overloaded{[&checker](subexprs x) {
                                for (auto v : x) {
                                  checker(v.value());
                                }
                              },
                              [&valiter](int x) {
                                CHECK(x == get<int>(*valiter));
                                valiter++;
                              },
                              [&valiter](symbol x) {
                                CHECK(x == get<symbol>(*valiter));
                                valiter++;
                              },
                              [&valiter](boolean x) {
                                CHECK(x.value == get<boolean>(*valiter).value);
                                valiter++;
                              },
                              [](auto x [[gnu::unused]]) { FAIL(); }},
                   x.value());
      };
      checker(i->value());
    }
  }
  SECTION("ignore comments", "[constructor]") {
    Reader r(tokenizer("(1 (2 ;# thi )))\n 3) 4)"));
    subexprs e = std::get<subexprs>(read_form(r));
    std::vector<std::variant<int, symbol, boolean>> expected{1, 2, 3, 4};
    CHECK(e.size() == 3);
    CHECK(std::get<subexprs>((++e.begin())->value()).size() == 2);
    auto valiter = expected.begin();
    for (auto i = e.begin(); i != e.end(); i++) {
      std::function<void(expression)> checker;
      checker = [&checker, &valiter](expression x) {
        std::visit(overloaded{[&checker](subexprs x) {
                                for (auto v : x) {
                                  checker(v.value());
                                }
                              },
                              [&valiter](int x) {
                                CHECK(x == get<int>(*valiter));
                                valiter++;
                              },
                              [](auto x [[gnu::unused]]) { FAIL(); }},
                   x.value());
      };
      checker(i->value());
    }
  }
}

TEST_CASE("Check read_string ignore comments in math", "[constructor]") {
  read_string(R"((+ 1 (+ 2 ;# thi )))
3)))");
}

TEST_CASE("Check READ") {
  SECTION("read from stream") {
    std::istringstream in("1");
    auto e = READ(in);
    CHECK(to_string(std::get<expression>(e).value()) == "1");
  }

  SECTION("Check read from stream multiline") {
    std::istringstream in("(1\n2)");
    auto e = READ(in);
    CHECK(to_string(
              std::get<expression::subexprs>(std::get<expression>(e).value())
                  .front()
                  .value()) == "1");
  }

  SECTION("Check read from stream multiline with comment in form ") {
    std::istringstream in("(1 ; hello\n2)");
    auto e = READ(in);
    CHECK(to_string(
              std::get<expression::subexprs>(std::get<expression>(e).value())
                  .front()
                  .value()) == "1");
    CHECK(to_string(
              (++std::get<expression::subexprs>(std::get<expression>(e).value())
                     .begin())
                  ->value()) == "2");
  }

  SECTION("Check read from stream multiline with comment after form") {
    std::istringstream in("(1 \n2) ; hello");
    auto e = READ(in);
    CHECK(to_string(
              std::get<expression::subexprs>(std::get<expression>(e).value())
                  .front()
                  .value()) == "1");
    CHECK(to_string(
              (++std::get<expression::subexprs>(std::get<expression>(e).value())
                     .begin())
                  ->value()) == "2");
  }

  SECTION("Check read on unclosed paren", "[constructor]") {
    std::istringstream in("(");
    CHECK_THROWS(READ(in));
  }

  SECTION("Check read on only closed paren", "[constructor]") {
    std::istringstream in("(");
    CHECK_THROWS(READ(in));
  }

  SECTION("Check read on only open paren with comment", "[constructor]") {
    std::istringstream in("( ;");
    CHECK_THROWS(READ(in));
  }

  SECTION("Check read on only open paren with comment and new line",
          "[constructor]") {
    std::istringstream in("( ;\n");
    CHECK_THROWS(READ(in));
  }

  SECTION("Check read on only hash", "[constructor]") {
    std::istringstream in("#");
    CHECK_THROWS(READ(in));
  }

  SECTION("Check read on empty line", "[constructor]") {
    std::istringstream in("");
    auto e = READ(in);
    CHECK(std::get<Reader_Responses>(e) == END_OF_FILE);
  }

  SECTION("Check read only new line", "[constructor]") {
    std::istringstream in("\n");
    auto e = READ(in);
    CHECK(std::get<Reader_Responses>(e) == END_OF_FILE);
  }

  SECTION("Check read reading comment with new lins", "[constructor]") {
    std::istringstream in(";; hello\n");
    auto e = READ(in);
    CHECK(std::get<Reader_Responses>(e) == EMPTY_LINE);
  }

  SECTION("Check read whitespace empty line", "[constructor]") {
    std::istringstream in(" ");
    auto e = READ(in);
    CHECK(std::get<Reader_Responses>(e) == EMPTY_LINE);
  }

  SECTION("Check read ignore line comments nested", "[constructor]") {
    std::istringstream in(";; hello");
    auto e = READ(in);
    CHECK(std::get<Reader_Responses>(e) == EMPTY_LINE);
  }

  SECTION("Check read ignore line comments", "[constructor]") {
    std::istringstream in("; hello");
    auto e = READ(in);
    CHECK(std::get<Reader_Responses>(e));
  }
}
TEST_CASE("Check read_form handle built in symbols", "[constructor]") {
  auto t = tokenizer("(- * = (+ 3) 4)");
  Reader r(t);
  subexprs e = std::get<subexprs>(read_form(r));
  std::vector<std::variant<int, symbol, boolean>> expected{"-", "*", "=",
                                                           "+", 3,   4};
  CHECK(e.size() == 5);
  auto valiter = expected.begin();
  for (auto i = e.begin(); i != e.end(); i++) {
    std::function<void(expression)> checker;
    checker = [&checker, &valiter](expression x) {
      std::visit(overloaded{[&checker](subexprs x) {
                              for (auto v : x) {
                                checker(v.value());
                              }
                            },
                            [&valiter](int x) {
                              CHECK(x == get<int>(*valiter));
                              valiter++;
                            },
                            [&valiter](symbol x) {
                              CHECK(x == get<symbol>(*valiter));
                              valiter++;
                            },
                            [](auto x [[gnu::unused]]) { FAIL(); }},
                 x.value());
    };
    checker(i->value());
  }
}
TEST_CASE("Check to_string with sub lists", "[constructor]") {
  auto exp = "(1 (2 (1 9 3)) 4)";
  auto t = tokenizer(exp);
  Reader r(t);
  CHECK(to_string(read_form(r)) == exp);
}

TEST_CASE("Check unclosed expression", "[constructor]") {
  auto exp = "(1 (2 (1 9 3) 4)";
  auto t = tokenizer(exp);
  Reader r(t);
  CHECK_THROWS(read_form(r));
}

// The grammar I made actually says we don't support this
// TEST_CASE("Check cabobed symbol", "[constructor]") {
//   CHECK(to_string(read_string("hello-world").value()) == "hello-world");
// }

// TEST_CASE("Check ending in - symbol", "[constructor]") {
//   CHECK(to_string(read_string("hello-world").value()) == "hello-world");
// }

// TODO proper alpha numerics
TEST_CASE("Alpha Numerics") {
  SECTION("Check alpha numerics in list", "[constructor]") {
    CHECK(to_string(read_string("( a1 )").value()) == "(a1)");
  }

  SECTION("Check alpha numerics", "[constructor]") {
    CHECK(to_string(read_string("( b1c)").value()) == "(b1c)");
  }

  SECTION("Check alpha numerics sym", "[constructor]") {
    CHECK(to_string(read_string("(   ay1 )").value()) == "(ay1)");
  }

  SECTION("Check alpha numerics sym with num at start", "[constructor]") {
    CHECK_THROWS(to_string(read_string("1ay1").value()));
  }
}

TEST_CASE("Check alpha numerics in list") {
  CHECK(to_string(read_string("a1-").value()) == "a1");
}

TEST_CASE("Always invalid input ") {
  SECTION("invalid symbol") {
    CHECK_THROWS(to_string(read_string("<").value()));
  }
  SECTION("multiple hashes") {
    CHECK_THROWS(to_string(read_string("##").value()));
  }
  SECTION("hashes on other than tru") {
    CHECK_THROWS(to_string(read_string("#b").value()));
  }
}

TEST_CASE("Check Cons Cells") {
  SECTION("reading cons on sym expressions") {
    CHECK_NOTHROW(to_string(read_string("(1. 2 )").value()));
    CHECK(to_string(read_string("(1. 2 )").value()) == "(1 . 2)");
  }

  SECTION("reading cons expressions no space") {
    auto expers = std::get<expression::cons>(read_string("(1 . 2 )").value());
    CHECK(std::get<int>(expers->first.value()) == 1);
    CHECK(std::get<int>(expers->second.value()) == 2);
    CHECK(to_string(expers) == "(1 . 2)");
  }

  SECTION("reading cons expressions nested") {
    CHECK(to_string(read_string("((1 . 2 ))").value()) == "((1 . 2))");
  }

  SECTION("reading cons expressions in list ") {
    CHECK(to_string(read_string("( 2 (1 . 2 ))").value()) == "(2 (1 . 2))");
  }
  SECTION("reading cons expressions") {
    auto expers =
        std::get<expression::subexprs>(read_string("( 2 (1 . 2 ))").value());
    CHECK(std::get<int>(expers.front().value()) == 2);
    CHECK(to_string(std::get<expression::cons>((++expers.begin())->value())) ==
          "(1 . 2)");
  }
  SECTION("reading cons expressions with cons of conses") {
    CHECK(to_string(read_string("( (   a . hello  ) . ( 1 . 2 ))").value()) ==
          "((a . hello) . (1 . 2))");
  }
  SECTION("Reading cons with unclosed paren") {
    CHECK_THROWS(to_string(read_string("( (   a .  . ( 1 . 2 ))").value()));
  }
  // This is the cause of the leak
  SECTION("Reading cons with nothing before") {
    CHECK_THROWS(to_string(read_string("( ( . b  . ( 1 . 2 ))").value()));
  }
  SECTION("Reading cons with nothing before single") {
    CHECK_THROWS(to_string(read_string("( . b").value()));
  }
  SECTION("Reading cons with no close ") {
    CHECK_THROWS(to_string(read_string("( a . b").value()));
  }
  SECTION("Reading cons with no 2nd val with paren") {
    CHECK_THROWS(to_string(read_string("( a . )").value()));
  }

  SECTION("Reading cons with no open 2nd val and end of reader") {
    CHECK_THROWS(to_string(read_string("( a .").value()));
  }
  SECTION("Reading cons with no open 2nd val and not end of reader") {
    CHECK_THROWS(to_string(read_string("( a . ").value()));
  }
  SECTION("multiple atoms in cons") {
    CHECK_THROWS(to_string(read_string("(1 a . c)").value()));
  }
  SECTION("READ atoms in cons") {
    std::istringstream in("(quote ( 1 2 . 3) )");
    CHECK_THROWS(READ(in));
  }
  SECTION("READ atoms extra after dot") {
    std::istringstream in("(quote ( 1 . 3 4) )");
    CHECK_THROWS(READ(in));
  }
}
