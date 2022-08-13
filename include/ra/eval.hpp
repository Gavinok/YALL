#ifndef EVAL
#define EVAL

#include <map>
#include "ra/read.hpp"
class environment;

expression::sexpr& tru_eval(expression& expr, environment& env);
void validate_argument_count(size_t expected, size_t given);

class environment {
public:
  using sexpr = expression::sexpr;
  using subexprs = expression::subexprs;
  using args = subexprs::iterator;
  using args_size = size_t;
  // TODO map of symbols?
  environment();
  // XXX This use of a raw pointer should be replaced with something easier to manage
  environment(const environment& e): bindings(e.bindings) {
    DBG("constructing environment with outerscope");
  };

  ~environment(){
    DBG("Destructing environment");
  }
  std::function<int(args)>& constructArgs(symbol s);
  sexpr& get(symbol s);
  sexpr& set(symbol s, sexpr expr);
private:
  // Representation of the outer environments. Specifically intended to be used with `let`
  // XXX This use of a raw pointer should be replaced with something easier to manage
  // std::list<std::shared_ptr<std::map<symbol, sexpr>>> outer_scopes;
  // Default environment bindings
  std::map<symbol, sexpr> bindings;
};

expression::sexpr& eval_subexpressions(expression& expr, environment& env);
expression::sexpr expand(expression::sexpr s, environment env);
#endif
