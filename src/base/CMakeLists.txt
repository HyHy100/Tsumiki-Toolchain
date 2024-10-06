add_library(base)

target_sources(
    base
    PRIVATE
    rtti.cc
)

target_include_directories(
    base
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)