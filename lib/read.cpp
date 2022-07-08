#include "ra/read.hpp"
#include <iostream>

// TODO consider changing this to a class later
using symbol = std::string;
using sexpr = expression::sexpr;

#ifdef MY_DEBUG
#define DBG(X) { std::cout << X << std::endl}
#else
#define DBG(X) {}
#endif
void die(std::string x, int exit_code){
  std::cerr << x << std::endl;
  exit(exit_code);
}

expression::expression(): exprs() {};
// TODO Delete subexpressions
expression::~expression() = default;
void expression::open () {
  DBG("opened");
  state = OPEN;
}
void expression::close () { state = CLOSED;}
bool expression::expressionStarted(){
  return state == OPEN ? true : false;
}
std::vector<expression::sexpr> expression::expressions() const { return exprs; }

void expression::pushSymbol(const symbol& x){
  if (expressionStarted())
    return exprs.push_back(sexpr(x));
  die("Symbol before opening paren", 1);
}

void expression::push_subexpression(expression* e){
  DBG("opened sub expression");
  // if (expressionStarted())
  return exprs.push_back(new expression(*e));
  // die("Symbol before opening paren", 1);
}

std::ostream &operator<<(std::ostream &os, const expression& expr) {
  auto print_sym = [&os](const symbol sy){ os << sy; };
  auto print_expr = [&os](const expression* expr){ os << "\n\t" <<  *expr; };
  // TODO determin why it says it has more elemnts than it should
  DBG("there are crurrently " << expr.expressions().size() << " expressions\n");
  os << "(";
  bool first = true;
  for(auto expr : expr.expressions()){
    if (!first)
      os << " ";
    std::visit(overloaded{print_sym, print_expr}, expr);
    first = false;
  }
  os << ")";
  return os;
}

expression* READ(std::string str) {
  return READ(str.begin(), str.end());
}

expression* READ(std::string::const_iterator iter, std::string::const_iterator end) {
  expression* exp = new expression;
  std::optional<symbol> current_symbol = std::nullopt;
  for(; iter != end; iter++){
    DBG("current char is ");
    if (current_symbol)
      DBG("current symbol is " << current_symbol.value());

    // START OF EXPRESSION
    if (*iter == '('){
      // Ensure the current symbol is closed if there is once
      if (auto sy = current_symbol){
        exp->pushSymbol(sy.value());
        current_symbol.emplace(symbol(""));
      }
      /* start of expression */;
      if (!exp->expressionStarted()){
        exp->open();
        DBG("opened");
        continue;
      }
      if (exp->expressionStarted()){
        exp->push_subexpression(READ(iter, end));
	DBG("opened");
	// skip to the end of the subexpression
	for (; *iter != ')'; iter++)
	  ;
        continue;
      }
      // TODO handle sub expressions
    }

    // END OF EXPRESSION
    if (*iter == ')'){
      // End the current symbol
      if (auto sy = current_symbol){
        exp->pushSymbol(sy.value());
        current_symbol.emplace(symbol(""));
      }
      if (exp->expressionStarted())
        exp->close();
      DBG("closed");
      /* END OF EXPRESSION */
      return exp;
    }

    // SYMBOL TOKENS
    if (*iter == ' '){
      if (auto sy = current_symbol){
        exp->pushSymbol(sy.value());
        current_symbol.emplace(symbol(""));
      }
      DBG("end/start of symbol");
      continue;
    }

    // STORE PART OF CURRENT SYMBOL
    if (current_symbol)
      current_symbol->push_back(*iter);
    else
      current_symbol.emplace(symbol{*iter});
  }
  return exp;
};
