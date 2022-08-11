#include "ra/eval.hpp"
#include "ra/read.hpp"
using subexprs = expression::subexprs;
using lisp_function = expression::lisp_function;
using sexpr = expression::sexpr;

// sexpr& environment::get(symbol s){
//   DBG("Looking up binding for " + pr_str(s));
//   return bindings.at(s);
// }
sexpr& environment::get(symbol s){
  try {
    return bindings.at(s);
  } catch (...){
    // TODO catch the exact exception when the binding can not be resolved
    if (outer_scopes)
      return outer_scopes->get(s);
  }
  DBG("Looking up binding for " + pr_str(s));
  throw std::runtime_error("Could not resolve binding for " + pr_str(s));
}
sexpr& environment::set(symbol s, sexpr expr){
  DBG("Defining binding for up binding for " + pr_str(s));
  return bindings[s] = expr;
};

sexpr create_lambda(subexprs& sub_expressions, environment& env){
  DBG("It's a lambda definition");

  auto arg_symbols = std::get<subexprs>(sub_expressions.at(1).value());
  auto body = sub_expressions.at(2);

  DBG("Constructing Lambda");

  // TODO passing by value is gonna be a bottleneck
  auto lambda = [env, body, arg_symbols](environment::args lambda_args) -> sexpr {
    environment lambda_env(env);

    DBG("Getting lambda list");
    DBG("lambda list found");

    auto arg_expers = arg_symbols;
    auto expr = body;

    // Ensure proper number of arguments are passed
    if (arg_symbols.size() != lambda_args.size())
      throw std::runtime_error ("Wrong number of arguments passed. Expected "
                                + std::to_string(lambda_args.size()));

    // Bind arguments to the current environment
    for (auto sym = arg_expers.begin(), arg = lambda_args.begin();
         arg != lambda_args.end() && sym != arg_expers.end();
         ++sym, ++arg) {

      DBG("lambda list determining argument symbols");
      DBG("binding " + pr_str((*sym).value()));

      lambda_env.set(std::get<symbol>((*sym).value()),
                     tru_eval(*arg , lambda_env));

      DBG("lambda list symbols are now bound");

    }
    return tru_eval(expr , lambda_env);
  };

  DBG("New lambda defined");

  return lambda;
}
sexpr& tru_eval(expression& expr, environment& env){
  expr = expression(std::visit(overloaded{
        [&expr, &env](subexprs& sub_expressions) -> sexpr {
          DBG("Determined to be a sub expression");

          // EMPTY LIST
          if(sub_expressions.size() == 0) return sub_expressions;
          DBG("Checking the first of the expressions" + pr_str(sub_expressions.front().value()));

          return std::visit(overloaded{
              [&env, &sub_expressions](subexprs nested_function_call [[gnu::unused]]) -> sexpr {
                DBG("determined to be yet another expression there for it's a nested function call");
                auto func = std::get<lisp_function>(tru_eval(sub_expressions.front(), env));
                return func(std::vector(sub_expressions.begin()+1, sub_expressions.end())).value();
              },
              [&env, &sub_expressions, &expr](symbol element) -> sexpr {
                DBG("First arg was a symbol")

                  // SPECIAL FORMS
                  // Defining new variables
                  if (element == "define"){ // (define <symbol> <symbolic-expression>)
                    DBG("Defining new binding");
                    symbol symbol_to_bind = std::get<symbol>(sub_expressions.at(1).value());
                    env.set(symbol_to_bind, tru_eval(sub_expressions.at(2), env));
                    return sub_expressions.at(2).value();
                  }

                // Let blocks
                if (element == "let"){  // (let (<let-bindings>+) <symbolic-expression>)
                  DBG("Creating a new let block");
                  environment let_env(env);
                  subexprs let_bindings = std::get<subexprs>(sub_expressions.at(1).value());
                  for(auto& binding: let_bindings){
                    subexprs binding_args = std::get<subexprs>(binding.value());
                    symbol var = std::get<symbol>(binding_args.at(0).value());
                    let_env.set(var, tru_eval(binding_args.at(1), env));
                  }
                  return tru_eval(sub_expressions.at(2), let_env);
                }

                // Quoting
                if (element == "quote"){ // (quote <symbolic-expression>)
                  DBG("Quoted this ");
                  return sexpr(quoted<expression>{ std::make_shared<expression>(sub_expressions.at(1)) });
                }

                // Closures
                if (element == "lambda"){ // (lambda <lambda-list> <forms>+)
                  return create_lambda(sub_expressions, env);
                };

                // BASIC EXPRESSIONS
                auto expressions = std::get<subexprs>(eval_subexpressions(expr, env));
                auto func = std::get<lisp_function>(expressions.front().value());
                DBG("Function was found for " << pr_str(expressions.front().value()));
                DBG("executing functoin now");
                return func(std::vector(expressions.begin()+1, expressions.end())).value();

              },
              [](auto anything_else [[gnu::unused]]) -> sexpr {
                throw std::runtime_error("failed to properly evaluate this expression ");
              }
              },
            sub_expressions.front().value());
        },
        [&expr, &env](auto& anything_else [[gnu::unused]]) -> sexpr {
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
