#ifndef EVALH
#define EVALH

#include <map>
#include <memory>
#include "ra/read.hpp"

class environment {
  using sexpr = expression::sexpr;
  using args = std::vector<expression>;
public:
  // TODO map of symbols?
  environment(){}
  ~environment(){
    DBG("Destructing environment");
  }
  std::function<int(args)>& constructArgs(symbol s);
  sexpr& get(symbol s);
  sexpr& set(symbol s, sexpr expr);
// private:
  std::map<symbol, sexpr> bindings{
    {"+" , [](args operands) -> sexpr {
      return std::get<int>(operands[0].value()) + std::get<int>(operands[1].value());
    }},
    {"-" , [](args operands) -> sexpr{
      return sexpr(std::get<int>(operands[0].value()) - std::get<int>(operands[1].value()));
    }},
    {"*" , [](args operands) -> sexpr {
      return sexpr(std::get<int>(operands[0].value()) * std::get<int>(operands[1].value()));
    }},
    {"=" , [](args operands) -> sexpr {
      return sexpr(
                   boolean{
                     std::get<int>(operands[0].value()) == std::get<int>(operands[1].value())
                     }
                   );
    }},
    {"car" , [](args operands) -> sexpr {
      return sexpr(std::get<expression::subexprs>(operands.at(0).value()).front().value());
    }},
    {"cdr" , [](args operands) -> sexpr {
      auto tmp = std::get<expression::subexprs>(operands.at(0).value());
      return expression::subexprs(tmp.begin()+1, tmp.end());
    }}
  };
     {"eq" , [](args operands) -> sexpr {
       return sexpr(
                    boolean{
                      std::visit(overloaded{
                          [](int& a, int& b){ return a == b;},
                          // TODO properly compair symbols
                          [](symbol& a, symbol& b){ return a[0] == b[0];},
                          [](boolean& a, boolean& b){ return a.value == b.value; },
                          [](auto& a [[gnu::unused]], auto& b [[gnu::unused]]){ return false; }},
                        operands[0].value(),
                        operands[1].value())
                        });
     }},
};

expression::sexpr& eval_subexpressions(expression& expr, environment& env);
expression::sexpr& tru_eval(expression& expr, environment& env);
expression::sexpr expand(expression::sexpr s, environment env);
#endif
