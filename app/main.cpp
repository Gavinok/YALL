#include "ra/eval.hpp"
#include "ra/read.hpp"
#include <cstring>
#include <iostream>

// The default read-eval-print-loop
void REPL(bool prompt) {
  std::string line;

  auto prompter = [prompt](std::optional<std::string> result) {
    if (prompt) {
      if (result.has_value())
        std::cout << *result << std::endl;
      std::cout << "> ";
    }
  };

  std::optional<std::string> last_result;
  prompter(std::nullopt);

  environment env;
  // loop
  for (;;) {
    auto should_exit =
        std::visit(overloaded{
                       [&env, &prompter, &last_result](expression e) {
                         // A new form has been processed
                         last_result = yall::print(yall::eval(e, env));
                         prompter(last_result);
                         return false;
                       },
                       [&prompt, &last_result](Reader_Responses r) {
                         if (r == END_OF_FILE) {
                           if (!prompt && last_result.has_value())
                             std::cout << *last_result << std::endl;
                           return true;
                         }
                         return false;
                       },
                   },
                   yall::read(std::cin));
    if (should_exit)
      return;
  }
}

int main(int argc, char *argv[]) {
  try {
    if ((argc == 2) && ((strncmp(argv[1], "-i", 2) == 0) ||
                        (strncmp(argv[1], "--interactive", 13) == 0))) {
      REPL(true);
      return 0;
    }
    REPL(false);
  } catch (...) {
    std::cerr << "Undefined Behavior Encountered";
    return 1;
  }
  return 0;
}
