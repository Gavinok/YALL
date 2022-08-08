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
  for (auto c = str.begin(); c != str.end(); c++){
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

}

  }

}

    }

};
// TODO Delete subexpressions
expression::~expression() = default;

sexpr& expression::value () {
  return expr;
}


