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
buffer<read> buffer1: float;

buffer<read> buffer2: [][64]{
  a: float,
  b: float
};

struct Output {
  @location(0) color : float4,
  @location(1) normal: float4
}

@fragment
fn main(): Output {
  if buffer2[55][2] + 9 + 7 + 9 + 5 > 12 {
    66 + 88 + 99;
  }

  var a = 66;

  return Output(
    vec4f(1.0, 0.0, 0.0, 1.0),
    vec3f(1.0, 0.0, 0.0)
  );
})");

    return 0;
  }
}

int main(int argc, char* argv[]) {
  return kate::tlr::start(argc, argv);
}