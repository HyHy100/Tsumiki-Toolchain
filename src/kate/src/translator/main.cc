#include "parser.h"
#include "resolver.h"

#include "printers/glsl.h"

#include <fmt/format.h>

namespace kate::tlr {
  void error_callback(const std::string_view& message) {
    fmt::println("PARSER ERROR: {}", message);
  };

  int start(int argc, char* argv[]) {
    tlr::Parser parser(tlr::ParserOptions {
      .error_callback = error_callback
    });

    auto module = parser.parse(R"(struct VertexOutput {
      @location(0) position : float4,
      @location(1) normal: float3
    }

    @vertex
    fn vertex_main(
      @builtin(position) vertex_position : float3
    ): VertexOutput {
      return VertexOutput(
        float4(vertex_position, 1.0),
        float3(1.0)
      );
    }

    struct FragmentOutput {
      @location(0) color : float4,
      @location(1) normal : float3,
      @location(2) position : float4
    }

    fn generate_float4() : float4 {
      return float4(1.0);
    }

    @fragment
    fn fragment_main(
      @input fragment_input: VertexOutput
    ): FragmentOutput {
      var white_c : [55 + 99]int;
      white_c[66] += 55;

      return FragmentOutput(
        generate_float4(),
        fragment_input.normal,
        fragment_input.position
      );
    })");

    Resolver resolver;
    resolver.resolve(module.get());

    GLSLPrinter printer;
    printer.print(module.get());

    return 0;
  }
}

int main(int argc, char* argv[]) {
  return kate::tlr::start(argc, argv);
}