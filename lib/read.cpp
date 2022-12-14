#include "ra/read.hpp"
#include <iostream>
#include <stdexcept>

using symbol = std::string;
using sexpr = expression::sexpr;

/*
  Simply attempt to identify the different syntax elements that need
  to be differentiated before proper parsing can take place.

  For example "(hello)" =>  [ "(", "hello", ")" ]
  where [ ] represents a vector of tokens
*/
std::vector<token> tokenizer(std::string str) {
  using token = std::string;
  std::vector<token> tokens;
  std::string current_atom("");

  // Store the current value and reset the current atom to avoid
  // accidentally appending other symbols them.
  auto store_cur = [&tokens](std::string &current_atom) {
    if (!current_atom.empty()) {
      tokens.push_back(current_atom);
      current_atom.clear();
    }
  };

  // Store the current value and move to the next token
  auto store_cur_and_push = [&store_cur, &tokens](std::string &cur, token tok) {
    store_cur(cur);
    tokens.push_back(tok);
  };

  for (auto c = str.begin(); c != str.end(); ++c) {
    switch (*c) {
      // structure
    case '(':
      store_cur_and_push(current_atom, "(");
      break;
    case ')':
      store_cur_and_push(current_atom, ")");
      break;
    case '.':
      store_cur_and_push(current_atom, ".");
      break;
    case ';': // Comment
      store_cur_and_push(current_atom, ";");
      break;
      // Whitespace
    case '\n':
      store_cur_and_push(current_atom, "\n");
      break;
    case '+':
      store_cur_and_push(current_atom, "+");
      break;
    case '*':
      store_cur_and_push(current_atom, "*");
      break;
    case '=':
      store_cur_and_push(current_atom, "=");
      break;
    case '#':
      store_cur(current_atom);
      current_atom.push_back(*c);
      break;
    default:
      if (isspace(*c)) {
        store_cur(current_atom);
        break;
      }
      if (isalpha(*c) || isdigit(*c) || *c == '-') {
        current_atom.push_back(*c);
        break;
      } else
        throw std::invalid_argument("Symbols only support alpha numerics");
    }
  }
  store_cur(current_atom);
  return tokens;
}

/*
  Return the current token without iterating to the next entry in the
  reader.
 */
token Reader::peak() {
  if (iter == tokens_.end())
    throw std::runtime_error("End of reader on peak");
  return *iter;
}

/*
  Return the current token and iterating to the next entry in the
  reader.
 */
token Reader::next() {
  if (iter == tokens_.end())
    throw std::runtime_error("End of reader on next");
  token tmp = *iter;
  ++iter;
  return tmp;
};

/*
  read_cons attempts to read the contents of an cons cell with the pre
  condition that the reader has just passed a period. The additonal
  expression being passed must come from read_list to ensure that the
  reader is inside a set of parentheses. The first value must be
  stored in the list otherwise the cons cell is assumed to be
  malformed.

  Returns a new cons cell
 */
sexpr read_cons(Reader &r, expression::subexprs exprs) {
  using cpack = expression::cons_unpacked;
  DBG("Possible dot spotted spotted");
  // Check for invalid state where there was no symbol before this dot e.g. (.
  // a)
  if (exprs.size() != 1)
    throw std::invalid_argument("Failed to extract cons cell");

  DBG("Possible cons spotted");

  // skip the `.`
  r.next();

  DBG("looking at next");

  // store the first element from the expression to be used a cons cell
  expression fst = exprs.back();

  DBG("is empty now ");
  // extract the last element stored in the list
  auto a = expression(std::make_shared<cpack>(fst, read_form(r)));

  DBG("tuple finished");

  if (r.peak() == "\n") {
    r.next();
  }
  if (r.peak() != ")")
    throw std::invalid_argument("Failed to extract cons cell");

  // move past the closing parentheses
  r.next();
  return a.value();
}
/*
  read_list attempts to read the contents of an expression with the
  pre condition that the reader has just passed an opening
  parentheses.

  Returns a set of subexprs
 */
sexpr read_list(Reader &r) {
  using subexprs = expression::subexprs;
  r.next();
  DBG("value stored in list " + r.peak());
  subexprs exprs;
  while (r.peak() != ")") {
    if (r.peak() == ".") {
      return read_cons(r, exprs);
    } else
      // skip new lines in expressions
      if (r.peak() == "\n") {
        r.next();
      } else {
        exprs.push_back(read_form(r));
      }
  }
  r.next();
  return sexpr(exprs);
};

bool is_number(std::string s) {
  auto c = s.begin();

  // Check if the number starts
  if (s.size() > 1 && *c == '-')
    c++;

  for (; c != s.end(); c++) {
    if (!isdigit(*c))
      return false;
  }

  return true;
}
int to_number(std::string s) {
  DBG("reading as a number")
  return atoi(s.c_str());
}

bool is_symbol(std::string s) {
  // symbols have to start with a letter
  if (!std::isalpha(s.at(0)))
    return false;
  for (char &c : s) {
    if (!(std::isalpha(c) || isdigit(c) || c == '-'))
      return false;
  }
  return true;
}

symbol to_symbol(std::string s) {
  DBG("reading as a symbol")
  return s;
}

bool is_boolean(std::string s) {
  if (s.size() == 2 && s[0] == '#' && (s[1] == 'f' || s[1] == 't'))
    return true;
  return false;
}

boolean to_boolean(std::string s) {
  DBG("reading as a bool")
  if (s == "#t")
    return boolean{true};
  if (s == "#f")
    return boolean{false};
  throw std::invalid_argument("Could not determin type of boolean");
}

bool is_builtin(std::string s) {
  if (s.size() != 1)
    return false;

  switch (s[0]) {
  case '+':
    return true;
  case '-':
    return true;
  case '*':
    return true;
  case '=':
    return true;
  }

  return false;
}

/*
  read_atom takes the reader with the precondition that r.peak() will
  return an atom (an atom meaning some supported representation other
  than an expression or list). This is then returned as the atoms
  correspnding lisp value. e.g. if r.peak() returns "1" then it's
  converted into an integer.
 */
sexpr read_atom(Reader &r) {
  if (is_number(r.peak()))
    return to_number(r.next());
  if (is_symbol(r.peak()))
    return to_symbol(r.next());
  if (is_boolean(r.peak()))
    return to_boolean(r.next());
  if (is_builtin(r.peak()))
    return to_symbol(r.next());
  throw std::invalid_argument("Expression could not be matched to an atom " +
                              r.peak());
};

void skip_comment(Reader &r) {
  if (r.peak() == ";") {
    DBG("Comment found " << r.peak());
    while (r.next() != "\n")
      ;
  }
  r.peak();
}

/*
  Read will take the given reader and cont the tokenized contents into
  a sexpr (aka the internal representation for an expression).
 */
sexpr read_form(Reader &r) {
  DBG("Currently reading form");
  skip_comment(r);

  // Unless this is a comment disregard the newlines
  if (r.peak() == "\n") {
    r.next();
    return read_form(r);
  }

  DBG("Current val in read_form " << r.peak())
  if (r.peak() == "(")
    return read_list(r);
  return read_atom(r);
}

/*
  to_string will take a sexpr (some form of yall data type) and return
  it in a string representation. This is used by the P in REPL to
  print the current expression.
*/
std::string to_string(sexpr s) { return to_string(s, std::string("")); }
std::string to_string(sexpr s, std::string accum) {
  using str = std::string;
  using subexprs = expression::subexprs;
  using fn = expression::lisp_function;

  // This is the only one that requires us to capture the state of the
  // accumulator for recursion.
  auto subexpr_to_string = [&accum](subexprs &expressions) {
    str str_representation("(");
    for (auto &v : expressions) {
      str_representation += to_string(v.value(), accum) + " ";
    }

    if (str_representation.size() == 1)
      return str_representation + ')';

    str_representation.back() = ')';
    return str_representation;
  };

  auto numbers_to_string = [](int &x) -> str { return std::to_string(x); };

  auto symbol_to_string = [](symbol &x) -> str { return x; };

  auto func_to_string = [](fn &x [[gnu::unused]]) -> str {
    return "#<YALL Function>";
  };

  auto boolean_to_string = [](boolean &true_or_false) -> str {
    return true_or_false.value ? "#t" : "#f";
  };

  auto cons_to_string = [](expression::cons &cons) -> str {
    auto [fst, snd] = *cons;
    return "(" + to_string(fst.value()) + " . " + to_string(snd.value()) + ")";
  };

  auto quote_to_string = [](quoted<expression> &quoted_e) {
    return to_string((*(quoted_e.value)).value());
  };

  auto invalid_state = [](std::monostate a [[gnu::unused]]) -> str {
    throw std::invalid_argument("Cannot determin type when printing ");
  };

  return std::visit(overloaded{cons_to_string, invalid_state, subexpr_to_string,
                               quote_to_string, symbol_to_string,
                               numbers_to_string, boolean_to_string,
                               func_to_string},
                    s);
}

// The base type of an expression in the grammer this is represented
// by <symbolic-expression>
expression::expression(sexpr s) : expr(s) {
  DBG("Constructing Expression With " + to_string(s));
};

sexpr &expression::value() { return expr; }

expression read_string(std::string str) {
  Reader r(tokenizer(str));
  return expression(read_form(r));
}

/*
  The entry point to the REPL used to read an expression from a stream
  tokenize it and do some preliminary parsing.

  Finally this is returned as either an expression (indicating parsing
  could be done) or a Reader_Responses of either END_OF_FILE meaning
  that the end of the given stream was reached or EMPTY_LINE
  indicating that the last given line did not contain any express and
  evaluation can be skipped.
*/
std::variant<expression, Reader_Responses> yall::read(std::istream &is) {
  std::string expression_container;
  std::string accumulator;
  int open_parens = 0;
  while (std::getline(is, accumulator)) {
    open_parens = 0;

    if (accumulator.empty()) {
      DBG("Accumulator was empty");
      continue; // skip blank lines
    }

    expression_container += accumulator;
    Reader r(tokenizer(expression_container));
    DBG("Value in expression container is '" + expression_container + "'");

    try {
      // Skip blank lines early and comments early
      try {
        DBG("skip comment");
        skip_comment(r);
        DBG("done comment");
        if (r.peak() == "\n") {
          r.next();
        }
        DBG("done new  comment");
      } catch (...) {
        /*
          This line either only contained comments or was blank since
          the only exception that could have been thrown is from the
          reader
        */
        DBG("empty line ");
        return EMPTY_LINE;
      }

      DBG("Reading from");
      return read_form(r);

    } catch (std::runtime_error &e) {
      // This expression is not yet complete
      // handle unclosed parentheses
      ++open_parens;
      expression_container += "\n";
    }
  }
  if (open_parens > 0)
    throw std::runtime_error("End of file with open parentheses");

  // All open parens have been closed and an EOF has been recieved
  return END_OF_FILE;
}
std::string yall::print(expression::sexpr &e) { return to_string(e); };
