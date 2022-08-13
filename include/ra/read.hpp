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
// #include <vector>
#include <list>
#include <variant>
#include <memory>
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

/*
  Template class used to represent if a given expression should not be
  evaluated.
 */
template <class T>
struct quoted {
  using type = T;
  std::shared_ptr<T> value;
};

/*
  The general class used to represent an expression. An expression is
  really a wrapper for an std::variant of all possible data types
  supported by YALL.
 */
class expression {
public:
  // General representation of a list or function call
  // e.g. (+ 1 2) or (1 2)
  // Since in YALL and other lisps code is data
  using subexprs = std::list<expression>;

  /*
    Generalized function signature used by YALL. Since some YALL
    functions take variable arguments an iterator and it's size is
    passed to the function for per function handling.
  */
  using lisp_function = std::function<expression(subexprs::iterator, size_t)>;

  // Container representing a cons cell
  using cons_unpacked = std::pair<expression,expression>;
  using cons = std::shared_ptr<cons_unpacked>;

  /*
    The core representation of a type in YALL. Types are either a
    container of other YALL types such as a list or a primitive (from
    the perspective of YALL) type such as an integer.
  */
  using sexpr = std::variant
    <
    // Primitive Types
    symbol, lisp_function,  int, boolean,
    // Containers
    quoted<expression>, subexprs, cons
    >;

  expression(sexpr s);
  expression() = delete;
  ~expression() = default;

  // Return a reference to the expression contained within
  sexpr& value();

  /*
    Resolve the current expression to a given type. If the resolving
    to the given type is not possible an exception will be
    thrown. Intended for use in function calls
  */
  template<class T>
  T resolve_to(){
    return std::visit(overloaded{
        [](T a) -> T {return a;},
        [](quoted<expression> a) -> T {return std::get<T>((*a.value).value());},
        [](auto a [[gnu::unused]]) -> T {
          throw std::runtime_error("Could not resolve type");
        }
        },
      expr);
  }
private:
  sexpr expr;
};

using token = std::string;
class Reader{
public:
  Reader(std::vector<token> tokens): tokens_(tokens), iter(tokens_.begin()){};
  token peak();
  token next();
private:
  std::vector<token> tokens_;
  std::vector<token>::iterator iter;
};

std::vector<token> tokenizer(std::string str);
std::optional<expression> READ(std::istream& is);
expression read_string(std::string str);
expression::sexpr read_list(Reader& r);
expression::sexpr read_atom(Reader& r);
expression::sexpr read_form(Reader& r);
std::string to_string(expression::sexpr s);
std::string to_string(expression::sexpr s, std::string accum);

#endif
