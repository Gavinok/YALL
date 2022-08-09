#include "ra/eval.hpp"
#include "ra/read.hpp"
using subexprs = expression::subexprs;
using lisp_function = expression::lisp_function;
using sexpr = expression::sexpr;

sexpr& environment::get(symbol s){
  DBG("Looking up binding for " + pr_str(s));
  return bindings.at(s);
}
sexpr& environment::set(symbol s, sexpr expr){
  DBG("Defining binding for up binding for " + pr_str(s));
  return bindings[s] = expr;
};

sexpr& tru_eval(expression& expr, environment& env){
  expr = expression(std::visit(overloaded{
        [&expr, &env](subexprs& s) -> sexpr {
          // EMPTY LIST
          if(s.size() == 0) return s;
          if(const symbol* e = std::get_if<symbol>(&s.front().value())){
            // SPECIAL FORMS
            // Defining new variables
            if (*e == "define"){
              DBG("yep it's being defined");
              symbol x = std::get<symbol>(s[1].value());
              env.set(x, tru_eval(s.at(2), env));
              return s.at(2).value();
            }
            // TODO Allow let to search outer environments
            if (*e == "let"){
              DBG("yep it's being defined with let");
              environment let_env;
              subexprs let_bindings = std::get<subexprs>(s.at(1).value());
              for(auto& binding: let_bindings){
                subexprs binding_args = std::get<subexprs>(binding.value());
                symbol var = std::get<symbol>(binding_args.at(0).value());
                let_env.set(var, tru_eval(binding_args.at(1), env));
              }
              return tru_eval(s.at(2), let_env);
            }
            // Quoting
            if (*e == "quote"){
              DBG("Quoted this ");
              return s.at(1).value();
            }
            // BASIC EXPRESSIONS
            auto expressions = std::get<subexprs>(eval_subexpressions(expr, env));
            auto func = std::get<lisp_function>(expressions.front().value());
            DBG("Function was found for " << pr_str(expressions.front().value()));
            DBG("executing functoin now");
            return func(std::vector(expressions.begin()+1, expressions.end())).value();
          }
          throw std::runtime_error("failed to properly evaluate this expression ");
        },
        [&expr, &env](auto& s [[gnu::unused]]) -> sexpr {
          return eval_subexpressions(expr, env);
        }
        }, expr.value()));
  return expr.value();
}

sexpr& eval_subexpressions(expression& expr, environment& env){
  expr = expression(std::visit(overloaded{
      [&env](symbol& s) -> sexpr { return env.get(s); },
      [&env](subexprs& s) -> sexpr {
        auto exprs = s;
        subexprs accumulater;
        for(expression& e: exprs)
          accumulater.push_back(expression(expand(e.value(), env)));
        return sexpr(accumulater);
      },
      [](auto& a) -> sexpr {
        DBG("Unable to determin type");
        return a;
      }}
      , expr.value()));
  return expr.value();
}

sexpr expand(sexpr s, environment env){
  return std::visit(overloaded{
      [&env]    (subexprs& a)-> sexpr {
        auto e = expression(sexpr(a));
        return tru_eval(e, env);
      },
      [&env]    (symbol& a) -> sexpr { return env.get(a); },
      []        (auto& a)   -> sexpr { return a; }
      }
    ,s);
}
