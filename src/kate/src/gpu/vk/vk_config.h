#pragma once

#include <array>
#include <vulkan/vulkan.hpp>

namespace kate::gpu {
#   ifndef NDEBUG
    std::array<const char* const, 1> vulkan_validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    std::array<const char* const, 1> vulkan_instance_extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    std::array<const char* const, 1> vulkan_device_extensions = {};
#   else
    std::array<const char* const, 1> vulkan_validation_layers = {};

    std::array<const char* const, 1> vulkan_instance_extensions = {
        // ...
    };

    std::array<const char* const, 1> vulkan_device_extensions = {};
#   endif
}