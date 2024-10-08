#include "parser.h"

#include <fmt/format.h>

namespace kate::tlr {
  void error_callback(const std::string_view& message) {
      fmt::println("PARSER ERROR: {}", message);
    };

  int start(int argc, char* argv[]) {
    /*if (argc == 1) {
      std::cerr << "Missing shader source file." << std::endl;
      return 1;
    }*/

    tlr::Parser parser(tlr::ParserOptions {
      .error_callback = error_callback
    });

    parser.parse(R"(@group(0) @binding(0) 
buffer<read> buffer1: f32;
)");

    return 0;
  }
}

int main(int argc, char* argv[]) {
  return kate::tlr::start(argc, argv);
}