add_library(kate)

target_include_directories(kate PUBLIC include)

target_include_directories(kate PRIVATE include/kate)

target_sources(
    kate 
    PRIVATE 
    src/gpu/adapter.cc
    src/gpu/device.cc
    src/gpu/extent.cc
    src/gpu/memory.cc
)

set_target_properties(
    kate 
    PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)
