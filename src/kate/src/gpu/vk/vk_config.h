#pragma once

#include <array>
#include <vulkan/vulkan.hpp>

#ifdef __linux
#   include <X11/Xlib.h>
#   include <vulkan/vulkan_xlib.h>
#endif

#define VULKAN_DISABLE_VALIDATION_LAYERS

namespace kate::gpu {
    // enable validation layers unless explicitely disabled.
#   if !defined(NDEBUG) && !defined(VULKAN_DISABLE_VALIDATION_LAYERS)
    #define ENABLE_VALIDATION_LAYERS
#   endif

#   ifdef ENABLE_VALIDATION_LAYERS
    static inline std::array vulkan_validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
#   else
    static inline std::array<const char* const, 0> vulkan_validation_layers = {};
#   endif

#ifndef NDEBUG
    static inline std::array vulkan_instance_extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
#       ifdef __linux
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#       endif
    };

    static inline std::array<const char* const, 1> vulkan_device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#   else
    static inline std::array<const char* const, 0> vulkan_instance_extensions = {
        // ...
    };

    static inline std::array vulkan_device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#   endif

    constexpr vk::PresentModeKHR kVkDefaultPresentationMode = vk::PresentModeKHR::eFifo;
}