#ifndef EVALH
#define EVALH

#include <map>
#include <memory>
#include "ra/read.hpp"

class environment {
public:
  using sexpr = expression::sexpr;
  using subexprs = expression::subexprs;
  using args = subexprs::iterator;
  using args_size = size_t;
  // TODO map of symbols?
  environment(){}
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
  std::map<symbol, sexpr> bindings = std::map<symbol, sexpr>{
    {"+" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return resolve_to<int>(operands->value()) + resolve_to<int>((++operands)->value());
    }},
    {"-" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return resolve_to<int>(operands->value()) - resolve_to<int>((++operands)->value());
    }},
    {"*" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return resolve_to<int>(operands->value()) * resolve_to<int>((++operands)->value());
    }},
    {"=" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return boolean{
                resolve_to<int>(operands->value()) == resolve_to<int>((++operands)->value())
              };
    }},

        if (operands.size() == 3)
          return operands[2].value();

        else
          if (operands.size() == 2)
            return boolean{false};

      throw std::runtime_error("if expects at least 2 arguments");
    }},
    {"eq" , [](args operands) -> sexpr {
      // TODO properly compair symbols
      auto symbol_compair = [](const symbol& a, const symbol& b){ return a[0] == b[0];};
      return sexpr(
                   boolean{
                     std::visit(overloaded{
                           [](int& a, int& b){ return a == b;},
                           [](boolean& a, boolean& b){ return a.value == b.value; },
                           [&symbol_compair](quoted<expression>& a, quoted<expression>& b){
                             if (symbol_compair(std::get<symbol>((*(a.value)).value()),
                                                std::get<symbol>((*(b.value)).value())))
                               return true;
                             return false;
                           },
                           [](auto& a [[gnu::unused]],
                              auto& b [[gnu::unused]]){
                              return false;
                           }
                          },
                       operands[0].value(),
                       operands[1].value())
                       });
    }},
     {"car" , [](args operands) -> sexpr {
       auto e = (*(std::get<quoted<expression>>(operands.at(0).value()).value));
       return quoted<expression>{
         std::make_shared<expression>( std::get< expression::subexprs >( e.value() ).front().value() )
       };
     }},
     {"cdr" , [](args operands) -> sexpr {
       auto e = (*(std::get<quoted<expression>>(operands.at(0).value()).value)).value();
       return quoted<expression>{
         std::make_shared<expression>(expression::subexprs(std::get<expression::subexprs>(e).begin()+1,
                                                                                   std::get<expression::subexprs>(e).end()))
       };
     }}
    };
};

expression::sexpr& eval_subexpressions(expression& expr, environment& env);
expression::sexpr& tru_eval(expression& expr, environment& env);
expression::sexpr expand(expression::sexpr s, environment env);
#endif
