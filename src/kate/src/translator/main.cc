#include "lexer.h"
#include "ast.h"

#include <iostream>

namespace kate::tlr {
  int start(int argc, char* argv[]) {
    if (argc == 1) {
      std::cerr << "Missing shader source file." << std::endl;
      return 1;
    }

    tlr::Lexer lexer;
    lexer.tokenize(argv[1]);

    return 0;
  }
}

int main(int argc, char* argv[]) {
  return kate::tlr::start(argc, argv);
}