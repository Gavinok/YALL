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
    // TODO handle comments as a single token
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
    case ' ':
      store_cur(cur);
      break;
    case '\t':
      store_cur(cur);
      break;
      // Builtins
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
      if(isalpha(*c) || isdigit(*c)){
        cur.push_back(*c);
        break;
      } else
        throw std::runtime_error("symbols only support alpha numerics");
    }
  }
  store_cur(cur);
  return tokens;
}

// TODO add special case for comments
Reader::Reader(std::vector<token> tokens){
  tokens_ = tokens;
  iter = tokens_.begin();
};
token Reader::peak(){
  if (iter == tokens_.end())
    throw std::runtime_error("end of reader");
  return *iter;
}
token Reader::next(){
  if (iter == tokens_.end())
    throw std::runtime_error("end of reader");
  token tmp = *iter;
  ++iter;
  return tmp;
};
sexpr read_list(Reader& r){
  r.next();
  DBG("value stored in list " << r.peak());
  std::vector<expression> exprs;
  while(r.peak() != ")"){
    exprs.push_back(read_form(r));
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

sexpr read_form(Reader& r){
  if (r.peak() == ";"){
    DBG("Comment found " << r.peak());
    while (r.next() != "\n");
  }
  // Unless this is a comment disregard the newlines
  if(r.peak() == "\n") r.next();

  DBG("Current val in read_form " << r.peak())
  if (r.peak() == "(")
    return read_list(r);
  return read_atom(r);
}

// NOTE Default args seems to break recursion
std::string pr_str(sexpr s){ return pr_str(s, std::string("")); }
std::string pr_str(sexpr s, std::string accum) {
  using str = std::string;
  using subexprs = expression::subexprs;
  using fn = expression::lisp_function;
  auto subexpr_to_string = [&accum](subexprs x) {
    str s("(");
    for (auto v : x){
      s += pr_str(v.value(), accum) + " ";
    }
    s.back() = ')';
    return s;
  };

  // TODO this should not be necessary to call std::to_string
  auto numbers_to_string = [](int x) { return std::to_string(x); };
  auto symbol_to_string = [](symbol x) { return x; };
  auto func_to_string = [](fn x [[gnu::unused]]) -> str {
    return "#<YALL Function>";
  };
  auto boolean_to_string = [](boolean x) -> str {
    if (x.value) return "#t";
    else         return "#f";
  };
  return std::visit(overloaded
                    {
                      subexpr_to_string,
                      symbol_to_string,
                      numbers_to_string,
                      boolean_to_string,
                      func_to_string
                    }, s);
}

// The base type of an expression in the grammer this is represented
// by <symbolic-expression>
expression::expression(sexpr s) {
  expr = s;
};
// TODO Delete subexpressions
expression::~expression() = default;

sexpr& expression::value () {
  return expr;
}

expression read_string(std::string str){
  Reader r(tokenizer(str));
  return expression(read_form(r));
}

expression READ(std::string str) {
  Reader r (tokenizer(str));
  return expression(read_form(r));
}
