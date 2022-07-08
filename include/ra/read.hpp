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

enum expr_state {EMPTY, OPEN, READING_SYMBOL,  CLOSED};
class expression {
public:
  using sexpr = std::variant<std::string, expression*>;
  expression();
  expression(expression& e) = default;
  ~expression();
  void open ();
  void close ();
  bool expressionStarted();
  std::vector<sexpr> expressions() const;
  void pushSymbol(const symbol& x);
  void push_subexpression(expression* e);

private:
  expr_state state = EMPTY;
  std::vector<sexpr> exprs;
};
std::ostream &operator<<(std::ostream &os, const expression &expr);
expression* READ(std::string::const_iterator iter, std::string::const_iterator end);
#endif
