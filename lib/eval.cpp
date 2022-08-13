#include "ra/eval.hpp"
#include "ra/read.hpp"
using subexprs = expression::subexprs;
using lisp_function = expression::lisp_function;
using sexpr = expression::sexpr;

sexpr& environment::get(symbol s){
  DBG("Looking up binding for " + to_string(s));
  try {
  return bindings.at(s);
  } catch (...){
  //   if (outer_scopes)
  //     outer_scopes->get(s);
    throw std::runtime_error("Could not resolve binding for " + to_string(s));
  }
}

sexpr& environment::set(symbol s, sexpr expr){
  DBG("Defining binding for  " + to_string(s));
  return bindings[s] = expr;
};

void validate_argument_count(size_t expected, size_t given){
  if(given != expected)
    throw std::runtime_error("Expected " + std::to_string(expected) + " arguments"
                             "but got " + std::to_string(given));

}

sexpr create_lambda(subexprs sub_expressions, environment env){
  using args = environment::args;
  using args_size = environment::args_size;
  DBG("It's a lambda definition");

  auto lambda_iter = ++(sub_expressions.begin());
  auto arg_symbols = std::get<subexprs>(lambda_iter->value());
  auto body = *(++(lambda_iter));

  DBG("Constructing Lambda");

  // TODO passing by value is gonna be a bottleneck
  auto lambda = [env, body, arg_symbols](args lambda_args, args_size size) -> sexpr {
    environment lambda_env(env);

    DBG("Getting lambda list");
    DBG("lambda list found");

    auto arg_expers = arg_symbols;
    auto expr = body;

    // Ensure proper number of arguments are passed
    validate_argument_count(arg_symbols.size(), size);
    // Bind arguments to the current environment
    for (auto sym = arg_expers.begin(), arg = lambda_args;
         sym != arg_expers.end();
         ++sym, ++arg) {

      DBG("lambda list determining argument symbols");
      DBG("binding " + to_string((*sym).value()));

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
          if(sub_expressions.size() == 0) throw std::runtime_error("Invalid syntax in the expression ()");
          DBG("Checking the first of the expressions" + to_string(sub_expressions.front().value()));

          subexprs::iterator expr_iter = sub_expressions.begin();
          return std::visit(overloaded{
              [&env, &sub_expressions, &expr_iter](subexprs nested_fn_call [[gnu::unused]]) -> sexpr {
                DBG("determined to be yet another expression there for it's a nested function call");
                auto func = std::get<lisp_function>(tru_eval(*expr_iter, env));
                return func(++(expr_iter), sub_expressions.size()-1).value();
              },
              [&env, &sub_expressions, &expr, &expr_iter](symbol element) -> sexpr {
                DBG("First arg was a symbol")

                  // SPECIAL FORMS
                  // Defining new variables
                  if (element == "define"){ // (define <symbol> <symbolic-expression>)
                    DBG("Defining new binding");
                    validate_argument_count(2 , sub_expressions.size()-1);
                    auto fst = ++expr_iter;
                    auto snd = ++expr_iter;

                    symbol symbol_to_bind = std::get<symbol>(fst->value());
                    env.set(symbol_to_bind, tru_eval(*snd, env));
                    return snd->value();
                  }

                // Let blocks
                if (element == "let"){  // (let (<let-bindings>+) <symbolic-expression>)
                  DBG("Creating a new let block");
                  validate_argument_count(2 , sub_expressions.size()-1);
                  auto fst = ++expr_iter;
                  auto snd = ++expr_iter;

                  // TODO Create new environment for the let block that can fall back to the upper scope
                  environment let_env(env);
                  subexprs let_bindings = std::get<subexprs>(fst->value());
                  DBG("getting sub expressions")
                  for(auto& binding: let_bindings){
                    subexprs binding_args = std::get<subexprs>(binding.value());
                    validate_argument_count(2 , binding_args.size());
                    auto key = binding_args.begin();
                    auto value = ++(binding_args.begin());
                    symbol var = std::get<symbol>(key->value());
                    let_env.set(var, tru_eval(*value, env));
                  }
                  return tru_eval(*snd, let_env);
                }

                // Quoting
                if (element == "quote"){ // (quote <symbolic-expression>)
                  validate_argument_count(1 , sub_expressions.size()-1);
                  DBG("Quoted this ");
                  return sexpr(quoted<expression>{ std::make_shared<expression>(*(++expr_iter)) });
                }

                // Closures
                if (element == "lambda"){ // (lambda <lambda-list> <forms>+)
                  validate_argument_count(2 , sub_expressions.size()-1);
                  return create_lambda(sub_expressions, env);
                };

                // BASIC EXPRESSIONS
                auto expressions = std::get<subexprs>(eval_subexpressions(expr, env));
                auto func = std::get<lisp_function>(expressions.front().value());
                DBG("Function was found for " << to_string(expressions.front().value()));
                DBG("executing functoin now");
                return func(++(expressions.begin()), expressions.size() - 1).value();

              },
              [](auto anything_else [[gnu::unused]]) -> sexpr {
                throw std::runtime_error("failed to properly evaluate this expression ");
              }
              },
            expr_iter->value());
        },
        [&expr, &env](auto& anything_else [[gnu::unused]]) -> sexpr {
          return eval_subexpressions(expr, env);
        }
        }, expr.value()));
  return expr.value();
}

sexpr& eval_subexpressions(expression& expr, environment& env){
  expr = expression(std::visit(overloaded{
      [&env](symbol& sym) -> sexpr { return env.get(sym); },
      [&env](subexprs& expressions) -> sexpr {
        subexprs accumulater;
        for(expression& e: expressions){
          DBG("accumulating the value of expression " + to_string(e.value()));
          accumulater.push_back(expression(tru_eval(e, env)));
        }
        return sexpr(accumulater);
     },
     [](auto& anything_else) -> sexpr {
       DBG("Unable to determin type");
       return anything_else;
     }}
      , expr.value()));
  return expr.value();
}
