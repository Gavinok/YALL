#ifndef READF
#define READF
// #define MY_DEBUG
#ifdef MY_DEBUG
#define DBG(X) { std::cout << X << std::endl; }
#else
#define DBG(X) {}
#endif

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

// TODO Determin if this should just reference a value or if it should
// contain it.
// TODO determin how the actual primitive type should be obtained

struct boolean {
  bool value;
};
class expression {
public:
  // TODO replace int with a generalized type that can be used for anything
  using subexprs = std::vector<expression>;
  using lisp_function = std::function<expression(std::vector<expression>)>;
  using sexpr = std::variant<lisp_function,subexprs, int, symbol, boolean>;
  expression(sexpr s);
  expression(expression& e) = default;
  ~expression();
  expression(const expression& other) = default;
  sexpr& value();
  std::vector<sexpr> expressions() const;

private:
  sexpr expr;
};
using token = std::string;
class Reader{
public:
  Reader(std::vector<std::string> tokens);
  token peak();
  token next();
private:
  std::vector<token>::iterator iter;
  std::vector<token> tokens_;
};

std::vector<token> tokenizer(std::string str);
std::ostream &operator<<(std::ostream &os, const expression &expr);
expression READ(std::string s);
expression::sexpr read_list(Reader& r);
expression::sexpr read_atom(Reader& r);
expression::sexpr read_form(Reader& r);
std::string pr_str(expression::sexpr s);
std::string pr_str(expression::sexpr s, std::string accum);
#endif
