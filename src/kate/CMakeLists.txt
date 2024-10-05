add_library(kate)

target_include_directories(kate PUBLIC include)

target_include_directories(kate PRIVATE include/kate)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/khronos/vulkan-hpp khronos/vulkan-hpp)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/khronos/spirv-headers khronos/spirv-headers)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/khronos/vulkan-loader khronos/vulkan-loader)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/khronos/vulkan-utility-binaries khronos/vulkan-utility-binaries)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/khronos/spirv-tools khronos/spirv-tools)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/khronos/vulkan-validation-layers khronos/vulkan-validation-layers)

target_link_libraries(kate base vulkan SDL3-static)

target_sources(
    kate 
    PRIVATE 
    src/gpu/adapter.cc
    src/gpu/device.cc
    src/gpu/extent.cc
    src/gpu/queue.cc

    src/gpu/memory.cc

    src/gpu/vk/vk_adapter.cc
    src/gpu/vk/vk_device.cc
    src/gpu/vk/vk_texture.cc
    src/gpu/vk/vk_swapchain.cc
    src/gpu/vk/vk_queue.cc
)

set_target_properties(
    kate 
    PROPERTIES 
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED ON
)
