#include "ra/eval.hpp"
#include "ra/read.hpp"
#include <cstring>
#include <iostream>

// XXX Temperary hack just so I can get output form an rvalue
std::string PRINT(expression::sexpr& e){
  return pr_str(e);
};

bool getexpression(std::istream& is, std::string& expression_container){
  int parens = 0;
  expression_container.clear();
  std::string accumulator;
  while (std::getline(is, accumulator)){
    // Ignore blank lines
    if (accumulator.empty()) return true;
    for(auto c : accumulator){
      if (c == '(') ++parens;
      if (c == ')') --parens;
      DBG("there are this many parens left to match " + std::to_string(parens));
    }
    expression_container.append(accumulator);
    std::cout << "reading in" << expression_container << std::endl;
    if (parens == 0) return true;
  }
  return false;
}
// the default read-eval-print-loop
void REPL(bool prompt){
  std::string line;
  auto prompter = [prompt]{
    if (prompt)
      std::cout << "> ";
  };
  prompter();
  environment env;
  // loop
  // TODO Support multi line input
  int parens = 0;
  while (getexpression(std::cin, line)){
    if (line.empty()) continue;
    expression e = READ(line);
    std::cout << PRINT(tru_eval(e, env)) << std::endl;
    prompter();
  }
}

int main (int argc, char *argv[]){
  if ( (argc == 2) && ((strncmp(argv[1] ,"-i", 2) == 0)
                       ||(strncmp(argv[1] ,"--interactive", 13) == 0)) ){
    REPL(true);
    return 0;
  }
  REPL(false);
  return 0;
}
