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
using args = environment::args;
using args_size = environment::args_size;

// TODO
// Function Signature:  (cons <symbolic-expression> <symbolic-expression>)
auto yall_append (expression arg1, expression arg2) -> sexpr {
  auto lst = arg1.resolve_to<subexprs>();
  // We don't know if this is a list or just a value yet
  try {
    subexprs other_list = arg2.resolve_to<subexprs>();
    lst.insert(lst.end(), other_list.begin(), other_list.end());
    DBG(" it's a list '");
  } catch (...){
    DBG("Oh nooo")
      lst.push_back(arg2);
  }
  return quoted<expression>{
    std::make_shared<expression>( lst )
  };

}

// TODO
// Function Signature:  (cons <symbolic-expression> <symbolic-expression>)
auto yall_cons (expression fst, expression snd) -> sexpr {
  return std::visit(overloaded{
      [&fst](expression::cons cons) -> sexpr {
        DBG("Consing 2 cons cells together");
        return std::make_shared<expression::cons_unpacked>(fst, cons);
      },
      [&fst](quoted<expression> lst) -> sexpr{
        DBG("Consing a quoted expression together");
        return yall_cons(fst, *lst.value);
      },
      [&fst](expression::subexprs lst) -> sexpr{
        DBG("Consing 2 cons cells together");
        lst.push_front(fst);
        return lst;
      },
      [&fst](auto snd) -> sexpr {
        DBG("just going with the default");
        return std::make_shared<expression::cons_unpacked>(fst, snd);
      }
      }, snd.value());
}

// TODO
// Function Signature:  (car <list>)
auto yall_car (expression& list) -> sexpr {
  try{
    auto inner_list = list.resolve_to<subexprs>();
    if (inner_list.size() == 0) throw std::runtime_error("Car called on empty list");
    return quoted<expression>{
      std::make_shared<expression>( inner_list.front().value() )
    };
  }
  catch (...){
    DBG("It's not a list");
    auto [head, tail] = (*list.resolve_to<expression::cons>());
    return head.value();
  }
}

// TODO
// cdr will take a list or cons cell and return the repaining elements 
// Function Signature:  (cdr <list>)
auto yall_cdr (expression& list) -> sexpr {
  try{
    auto inner_list = list.resolve_to<subexprs>();
    if (inner_list.size() == 0) throw std::runtime_error("Cdr called on empty list");
    return quoted<expression>{
      std::make_shared<expression>(subexprs(++(inner_list.begin()), inner_list.end()))
    };
  }
  catch (...){
    DBG("It's not a list");
    auto [head, tail] = (*list.resolve_to<expression::cons>());
    return tail.value();
  }

}

// Since `list` takes variable arguments an iterator must be passed
// Function Signature:  (list <arg>+)
auto yall_list (args& list_elements, args_size size) -> sexpr {
  if(size < 1)
    throw std::runtime_error("Expected at least 1 arguments but got " + std::to_string(size));
  std::cout << "creating list " << std::endl;
  auto new_list = std::make_shared<expression>(subexprs{*list_elements});

  args_size count = 0;
  for (auto arg = (++list_elements); count < (size - 1); ++arg,++count)
    std::get<subexprs>(new_list->value()).push_back(*arg);
  return quoted<expression>{ new_list };

}

environment::environment():
  bindings({
    {"+" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return operands->resolve_to<int>() + (++operands)->resolve_to<int>();
    }},
    {"-" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return operands->resolve_to<int>() - (++operands)->resolve_to<int>();
    }},
    {"*" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return operands->resolve_to<int>() * (++operands)->resolve_to<int>();
    }},
    {"=" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      return boolean{
        operands->resolve_to<int>() == (++operands)->resolve_to<int>()
      };
    }},
    {"eq" , [this](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);

      auto fst = operands, snd = ++operands;

      // A bit of a hack but this seems to work the most consistantly
      return sexpr(boolean{ to_string(fst->value()) == to_string(snd->value()) });
    }},
    {"if" , [](args operands, args_size size) -> sexpr {
      if(size > 3 ||  size < 2)
        throw std::runtime_error("Expected 2 or 3 arguments but got " + std::to_string(size));
      auto condition = std::get<boolean>(operands->value());
      auto thn = ++operands;
      auto els = ++operands;
      return condition.value
        ? thn->value()
        : (size == 3) ? els->value() : boolean{false};
    }},
    {"empty" , [](args operands, args_size size) -> sexpr {
      if(size < 1)
        throw std::runtime_error("Expected at least 1 arguments but got " + std::to_string(size));
      subexprs inner_list = operands->resolve_to<subexprs>();
      return inner_list.empty() ? boolean{true} : boolean{false};
    }},
    {"list" , [](args operands, args_size size) -> sexpr {
      return yall_list(operands, size);
    }},
    {"append" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      auto fst = *operands;
      auto snd = *(++operands);
      return yall_append(fst, snd);
    }},
    {"cons" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      auto fst = *operands;
      auto snd = *(++operands);
      return yall_cons(fst, snd);
    }},
    {"car" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);
      return yall_car(*operands);
    }},
    {"cdr" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);
      return yall_cdr(*operands);
    }},
    {"sleep" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);

      auto time = operands->resolve_to<int>();
      sleep(time);
      return (*operands).value();
    }},
    {"print" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);

      std::cout << to_string(operands->value())  << "\n";
      return (*operands).value();
    }}
    // // TODO All of these need to be given the right environemnt
    // {"map" , [this](args operands, args_size size) -> sexpr {
    //   validate_argument_count(2, size);

    //   auto func = std::get<expression::lisp_function>(tru_eval(*operands, *this));
    //   auto e = (*( std::get<quoted<expression>>( (++operands)->value() ).value )).value();
    //   auto lst = std::get<subexprs>(e);

    //   subexprs accumulator;
    //   for(auto i : lst){
    //     auto args = subexprs{i};
    //     accumulator.push_back(func(args.begin(), args.size()));
    //   }

    //   return quoted<expression>{
    //     std::make_shared<expression>(accumulator)
    //   };
    // }},
    // {"filter" , [this](args operands, args_size size) -> sexpr {
    //   validate_argument_count(2, size);

    //   auto func = std::get<expression::lisp_function>(tru_eval(*operands, *this));
    //   auto e = (*(std::get<quoted<expression>>((++operands)->value()).value)).value();
    //   auto lst = std::get<subexprs>(e);

    //   subexprs accumulator;
    //   for(auto i : lst){
    //     // XXX expensive
    //     auto args = subexprs{sexpr(quoted<expression>{std::make_shared<expression>(i)})};
    //     if(std::get<boolean>(func(args.begin(), args.size()).value()).value)
    //       accumulator.push_back(i);
    //   }

    //   return quoted<expression>{
    //     std::make_shared<expression>(accumulator)
    //   };
    // }},
    // // TODO Handle lists better
    // {"foldl" , [this](args operands, args_size size) -> sexpr { //  (foldl )
    //   validate_argument_count(3, size);

    //   auto func = std::get<expression::lisp_function>(tru_eval(*operands, *this));
    //   auto accumulator = (++operands)->value();

    //   expression::sexpr e =
    //       (*(std::get<quoted<expression>>((++operands)->value()).value))
    //           .value();
    //   auto lst = std::get<subexprs>(e);

    //   for(auto i: lst){
    //     if (accumulator.index() == i.value().index())
    //       accumulator =
    //         std::visit(overloaded{
    //             [&func](auto & a1, auto& a2) -> sexpr {
    //               auto args = subexprs{sexpr(a1), sexpr(a2)};
    //               return func(args.begin(), args.size()).value();
    //             }},
    //           accumulator,
    //           i.value());
    //   }
    //   return quoted<expression>{
    //     std::make_shared<expression>(accumulator)
    //   };
    // }},
    // // Nice to haves
    // // TODO never said I'd make this avalilable to the user in the proposal but would be a nice to have
    // // todo can't find info from the outer environment
    // {"eval" , [this](args operands, args_size size) -> sexpr {
    //   validate_argument_count(1, size);
    //   std::cout << "eval this " + to_string(operands->value()) + "\n" << std::endl;
    //   expression holder = read_string(to_string(operands->value()));
    //   return tru_eval(holder, *this);
    // }},
    }){};
  

