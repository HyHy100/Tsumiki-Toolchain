cmake_minimum_required(VERSION 3.22)

project(TsumikiProject)

include_directories(third_party/khronos/vulkan-hpp/Vulkan-Headers/include)
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(kate)

add_executable(tsumiki)

set_target_properties(
    tsumiki
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(tsumiki kate)

target_sources(tsumiki PRIVATE main.cc)