#include "ra/read.hpp"

class environment {
public:
  // TODO map of symbols?
};

// TODO handle C-d
// the default read-eval-print-loop
void REPL(){
  // loop
  std::string line;
  for (;;) {
    std::cout << '>';
    std::getline(std::cin, line);
    // TODO EVAL
    std::cout << *READ(line.begin(), line.end()) << '\n';
  }
}

int main (int argc, char *argv[]){
  if ( (argc == 2) && (strncmp(argv[1] ,"--interactive", 13) == 0) ){
    REPL(true);
    return 0;
  }
  REPL(false);
  return 0;
}
