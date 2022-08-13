#include "ra/read.hpp"
#include <iostream>

// TODO consider changing this to a class later
using symbol = std::string;
using sexpr = expression::sexpr;

void die(std::string x, int exit_code){
  std::cerr << x << std::endl;
  exit(exit_code);
}
std::vector<token> tokenizer(std::string str){
  std::vector<token> tokens;
  std::string cur("");
  auto store_cur = [&tokens](std::string& cur){
    if (!cur.empty()){
      DBG("Current token in tokenizer " << cur);
      tokens.push_back(cur);
      cur = std::string("");
    }
  };
  auto store_cur_and_push = [&store_cur, &tokens](std::string& cur, std::string tok){
    store_cur(cur);
    tokens.push_back(tok);
  };
  for (auto c = str.begin(); c != str.end(); ++c){
    switch (*c){
      // structure
    case '(':
      store_cur_and_push(cur, "(");
      break;
    case ')':
      store_cur_and_push(cur, ")");
      break;
    case '.':
      store_cur_and_push(cur, ".");
      break;
    case ';': // Comment
      store_cur_and_push(cur, ";");
      break;
      // Whitespace
    case '\n':
      store_cur_and_push(cur, "\n");
      break;
    case '+':
      store_cur_and_push(cur, "+");
      break;
    case '-':
      store_cur_and_push(cur, "-");
      break;
    case '*':
      store_cur_and_push(cur, "*");
      break;
    case '=':
      store_cur_and_push(cur, "=");
      break;
    case '#':
      store_cur(cur);
      cur.push_back(*c);
      break;
    default:
      if (isspace(*c)){
        store_cur(cur);
        break;
      }
      if(isalpha(*c) || isdigit(*c)){
        cur.push_back(*c);
        break;
      } else
        throw std::runtime_error("Symbols only support alpha numerics");
    }
  }
  store_cur(cur);
  return tokens;
}

token Reader::peak(){
  if (iter == tokens_.end())
    throw std::runtime_error("end of reader on peak");
  return *iter;
}
token Reader::next(){
  if (iter == tokens_.end())
    throw std::runtime_error("end of reader on next");
  token tmp = *iter;
  ++iter;
  return tmp;
};
sexpr read_list(Reader& r){
  using subexprs = expression::subexprs;
  using cpack = expression::cons_unpacked;
  r.next();
  DBG("value stored in list " << r.peak());
  subexprs exprs;
  while(r.peak() != ")"){
    if(r.peak() == "."){
      // Check for invalid state e.g. (. a)
      if (exprs.size() != 1) throw std::runtime_error("Failed to extract cons cell");

      DBG("Possible cons spotted");
      // skip the `.`
      r.next();
      DBG("looking at next");

      expression fst = exprs.back();

      DBG("popping now ");
      exprs.pop_back();
      DBG("popped now ");

        DBG("is emptynow ");
        // extract the last element stored in the list
        auto a = expression(std::make_shared<cpack>(fst, read_form(r)));

        DBG("tuple finished");
        if (r.peak() != ")")
          throw std::runtime_error("Failed to extract cons cell");

        return a.value();
    } else {
      exprs.push_back(read_form(r));
    }
  }
  r.next();
  return sexpr(exprs);
};

bool is_number(std::string s){
  for(char& c: s){
    if(!std::isdigit(c))
      return false;
  }
  return true;
}
int to_number(std::string s){
  DBG("reading as a number")
  return atoi(s.c_str());
}

bool is_symbol(std::string s){
  for(char& c: s){
    if(!std::isalpha(c))
      return false;
  }
  return true;
}

symbol to_symbol(std::string s){
  DBG("reading as a symbol")
  return s;
}

bool is_boolean(std::string s){
  if (s.size() == 2 && s[0] == '#' && (s[1] == 'f' || s[1] == 't'))
    return true;
  return false;
}

boolean to_boolean(std::string s){
  DBG("reading as a bool")
  if ( s == "#t") return boolean{true};
  if ( s == "#f") return boolean{false};
  throw std::runtime_error("Could not determin type of boolean");
}

bool is_builtin(std::string s){
  if (s.size() != 1)
    return false;

  switch (s[0]){
  case '+': return true;
  case '-': return true;
  case '*': return true;
  case '=': return true;
  }

  return false;
}
sexpr read_atom(Reader& r){
  if (is_number(r.peak()))  return to_number(r.next());
  if (is_symbol(r.peak()))  return to_symbol(r.next());
  if (is_boolean(r.peak())) return to_boolean(r.next());
  if (is_builtin(r.peak())) return to_symbol(r.next());
  throw std::runtime_error("Expression could not be matched to an atom" + r.peak());
};
void skip_comment(Reader& r) {
  if (r.peak() == ";"){
    DBG("Comment found " << r.peak());
    while (r.next() != "\n");
  }
  r.peak();
}
sexpr read_form(Reader& r) {
  DBG("Currently reading form");
  skip_comment(r);

  // Unless this is a comment disregard the newlines
  if(r.peak() == "\n") {
    r.next();
    return read_form(r);
  }

  DBG("Current val in read_form " << r.peak())
  if (r.peak() == "(")
    return read_list(r);
  return read_atom(r);
}

// NOTE Default args seems to break recursion
std::string to_string(sexpr s){ return to_string(s, std::string("")); }
std::string to_string(sexpr s, std::string accum) {
  using str = std::string;
  using subexprs = expression::subexprs;
  using fn = expression::lisp_function;
  auto quote_to_string = [](quoted<expression>& quoted_e) {
    return to_string((*(quoted_e.value)).value());
  };
  auto subexpr_to_string = [&accum](subexprs& expressions) {
    str str_representation("(");
    for (auto& v : expressions){
      str_representation += to_string(v.value(), accum) + " ";
    }

    if (str_representation.size() == 1)
      return str_representation + ')';

    str_representation.back() = ')';
    return str_representation;
  };

  // TODO this should not be necessary to call std::to_string
  auto numbers_to_string =
    [](int& x)                 -> str { return std::to_string(x); };
  auto symbol_to_string  =
    [](symbol& x)              -> str { return x; };
  auto func_to_string    =
    [](fn& x [[gnu::unused]])  -> str { return "#<YALL Function>"; };
  auto boolean_to_string =
    [](boolean& true_or_false) -> str { return true_or_false.value ? "#t" : "#f";};
  auto cons_to_string =
    [](expression::cons& cons) -> str {
      auto [fst, snd] = *cons;
      return "(" + to_string(fst.value()) + " . " + to_string(snd.value()) + ")";
    };
  auto invalid_state =
    [](std::monostate a [[gnu::unused]]) -> str { throw std::runtime_error("Cannot determin type when printing "); };
  return std::visit(overloaded
                    {
                      cons_to_string,
                      invalid_state,
                      subexpr_to_string,
                      quote_to_string,
                      symbol_to_string,
                      numbers_to_string,
                      boolean_to_string,
                      func_to_string
                    }, s);
}

// The base type of an expression in the grammer this is represented
// by <symbolic-expression>
expression::expression(sexpr s): expr(s) {
  DBG("Constructing Expression With " + to_string(s));
};

expression::~expression() {
  DBG("Destructing expression")
};

sexpr& expression::value () {
  return expr;
}

expression read_string(std::string str){
  Reader r(tokenizer(str));
  return expression(read_form(r));
}

std::optional<expression> READ(std::istream& is) {
  std::string expression_container;
  std::string accumulator;
  int open_parens = 0;
  while (std::getline(is, accumulator)){
    open_parens = 0;

    if (accumulator.empty()) continue; // skip blank lines

    expression_container += accumulator;
    Reader r(tokenizer(expression_container));

    try {
      try {// Skip blank lines early and comments early
        skip_comment(r);
        if(r.peak() == "\n") {
          r.next();
        }
      } catch (...) { // this line either only contained comments or was blank
        // TODO more explicit version
        return std::nullopt;
      }

      return read_form(r);

    } catch (...){ // handle unclosed parentheses
      // TODO this should probably only catch the error associated with parenthesis
      ++open_parens;
      expression_container += "\n";
    }
  }
  if (open_parens > 0)
    throw std::runtime_error ("End of file with open parentheses");
  return std::nullopt;
}
