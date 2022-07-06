#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class environment {
public:
  // TODO map of symbols?
};

// TODO consider changing this to a class later
using symbol = std::string;

class expression {
public:
  void print(){
    std::cout << '(';
    for(auto symbol : symbols)
      std::cout << symbol;
    std::cout << '\n';
  }
private:
  std::vector<symbol> symbols;
};

// the default read-eval-print-loop
void REPL(){
  // loop
  for (;;) {
    std::cout << '>';
    std::string line;
    std::getline(std::cin, line);
    // TODO READ
    // TODO EVAL
    std::cout << line << '\n';
  }
}

int main (){
  REPL();
}
