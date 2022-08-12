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
    {"eq" , [this](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);

      auto fst = operands, snd = ++operands;

      // A bit of a hack but this seems to work the most consistantly
      return sexpr(boolean{ pr_str(fst->value()) == pr_str(snd->value()) });
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
      subexprs inner_list = resolve_to<subexprs>(operands->value());
      return inner_list.empty() ? boolean{true} : boolean{false};
    }},
    {"list" , [](args operands, args_size size) -> sexpr {
      // TODO change this to cons till the end and append an empty list
      if(size < 1)
        throw std::runtime_error("Expected at least 1 arguments but got " + std::to_string(size));
      std::cout << "creating list " << std::endl;
      auto new_list = std::make_shared<expression>(subexprs{*operands});

      args_size count = 0;
      for (auto arg = (++operands); count < (size - 1); ++arg,++count)
        std::get<subexprs>(new_list->value()).push_back(*arg);
      std::get<subexprs>(new_list->value()).push_back(expression(sexpr{"nil"}));

      return quoted<expression>{ new_list };
    }},
    {"append" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      subexprs inner_list = resolve_to<subexprs>(operands->value());
      args other = (++operands);
      try {
        subexprs other_list = resolve_to<subexprs>(other->value());
        inner_list.insert(inner_list.end(), other_list.begin(), other_list.end());
        DBG(" it's a list '");
      } catch (...){
        DBG("Oh nooo")
        inner_list.push_back(*other);
      }
      return quoted<expression>{
        std::make_shared<expression>( inner_list )
      };
    }},
    {"cons" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(2, size);
      subexprs inner_list = subexprs{operands->value()};
      args other = (++operands);
      try {
        subexprs other_list = resolve_to<subexprs>(other->value());
        inner_list.insert(inner_list.end(), other_list.begin(), other_list.end());
        DBG(" it's a list '");
      } catch (...){
        DBG("Oh nooo")
        inner_list.push_back(*other);
      }
      return quoted<expression>{
        std::make_shared<expression>( inner_list )
      };
    }},
    {"car" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);
      auto inner_list = resolve_to<subexprs>(operands->value());
      if (inner_list.size() == 0) throw std::runtime_error("Car called on empty list");
      return quoted<expression>{
        std::make_shared<expression>( inner_list.front().value() )
      };
    }},
    {"cdr" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);
      auto inner_list = resolve_to<subexprs>(operands->value());
      if (inner_list.size() == 0) throw std::runtime_error("Cdr called on empty list");
      return quoted<expression>{
        std::make_shared<expression>(subexprs(++(inner_list.begin()), inner_list.end()))
      };
    }},
    {"sleep" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);

      auto time = resolve_to<int>(operands->value());
      sleep(time);
      return (*operands).value();
    }},
    {"print" , [](args operands, args_size size) -> sexpr {
      validate_argument_count(1, size);

      std::cout << pr_str(operands->value())  << "\n";
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
    //   std::cout << "eval this " + pr_str(operands->value()) + "\n" << std::endl;
    //   expression holder = read_string(pr_str(operands->value()));
    //   return tru_eval(holder, *this);
    // }},
  };
};

expression::sexpr& eval_subexpressions(expression& expr, environment& env);
expression::sexpr expand(expression::sexpr s, environment env);
#endif
