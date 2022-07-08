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

int main (){
  REPL();
}
