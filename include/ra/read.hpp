#ifndef INTERVAL
#define INTERVAL

#include <cstddef>
#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <variant>
#include <optional>
using symbol = std::string;
// These overload templates are taken from
// https://en.cppreference.com/w/cpp/utility/variant/visit
// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class expression {
public:
  // TODO replace int with a generalized type that can be used for anything
  using subexprs = std::vector<expression>;
  using sexpr = std::variant<subexprs, int>;
  expression(sexpr s);
  expression(expression& e) = default;
  ~expression();
  expression(const expression& other) = default;
  sexpr& value();
  // void open ();
  // void close ();
  // bool expressionStarted();
  std::vector<sexpr> expressions() const;
  // void pushSymbol(const symbol& x);
  // void push_subexpression(expression* e);

private:
  sexpr expr;
};
std::ostream &operator<<(std::ostream &os, const expression &expr);
expression READ(std::string s);
#endif
