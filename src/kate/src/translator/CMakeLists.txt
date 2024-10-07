add_executable(ksc)

set_target_properties(
  ksc 
  PROPERTIES 
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

target_sources(
  ksc
  PRIVATE 
  lexer.cc
  ast.cc
  main.cc
  parser.cc
)

target_link_libraries(ksc base)