#include "parser.h"

#include <fmt/format.h>

namespace kate::tlr {
  void error_callback(const std::string_view& message) {
    fmt::println("PARSER ERROR: {}", message);
  };

  int start(int argc, char* argv[]) {
    tlr::Parser parser(tlr::ParserOptions {
      .error_callback = error_callback
    });

    parser.parse(R"(@group(0) @binding(0) 
buffer<read> buffer1: f32;

@compute
fn main() {
  if 6 + 9 + 7 + 9 + 5 > 12 {
    66 + 88 + 99;
  }
})");

    return 0;
  }
}

int main(int argc, char* argv[]) {
  return kate::tlr::start(argc, argv);
}