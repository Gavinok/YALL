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
    auto should_exit = std::visit(overloaded{
        [&env, &prompter](expression e){
          std::cout << PRINT(eval(e, env)) << std::endl;
          prompter();
          return false;
        },
        [](Reader_Responses r){
          if (r == END_OF_FILE)
            return true;
          return false;
        },
        }, READ(std::cin));
    if (should_exit)
      return;
  }
}

int main (int argc, char *argv[]){
  try {
    if ( (argc == 2) && ((strncmp(argv[1] ,"-i", 2) == 0)
                         ||(strncmp(argv[1] ,"--interactive", 13) == 0)) ){
      REPL(true);
      return 0;
    }
    REPL(false);
  } catch (...){
    std::cerr << "Undefined Behavior Encountered";
    return 1;
  }
  return 0;
}
