#include "ra/eval.hpp"
#include "ra/read.hpp"
#include <cstring>
#include <iostream>


// XXX Temperary hack just so I can get output form an rvalue
std::string PRINT(expression e){
  return pr_str(e.value());
};

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
  while (std::getline(std::cin, line)){
    // TODO eval
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
