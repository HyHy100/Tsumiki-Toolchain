add_executable(ksc)

set_target_properties(
  ksc 
  PROPERTIES 
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

target_compile_options(ksc PRIVATE -fdiagnostics-color=always)

target_sources(
  ksc
  PRIVATE 
  lexer.cc
  ast.cc
  sem.cc
  types.cc
  main.cc
  parser.cc
  resolver.cc
)

target_link_libraries(ksc base fmt)