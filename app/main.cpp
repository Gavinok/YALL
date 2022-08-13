#include "ra/eval.hpp"
#include "ra/read.hpp"
#include <cstring>
#include <iostream>

// XXX Temperary hack just so I can get output form an rvalue
std::string PRINT(expression::sexpr& e){
  return to_string(e);
};

// The default read-eval-print-loop
void REPL(bool prompt){
  std::string line;
  auto prompter = [prompt]{
    if (prompt)
      std::cout << "> ";
  };
  prompter();
  environment env;
  // loop
  for (;;){
    if (auto e = READ(std::cin)){
      std::cout << PRINT(tru_eval(*e, env)) << std::endl;
      prompter();
    } else {
      break;
    }
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
