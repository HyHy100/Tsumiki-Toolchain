#include "vk_adapter.h"
#include "vk_device.h"
#include "vk_config.h"

#include <iostream>

namespace kate::gpu {
    VkDebugUtilsMessengerEXT debugMessenger;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void setupDebugMessenger(vk::Instance& instance) {
#       ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
#       endif
    }

    VkAdapterObject::VkAdapterObject()
    {
        vk::ApplicationInfo appinfo = vk::ApplicationInfo(
            "",                             // Application name
            vk::makeVersion(1, 0, 0),       // Application version
            "kate",                         // Engine name
            vk::makeVersion(0, 1, 0),       // Engine version
            vk::makeApiVersion(1, 0, 0, 0)  // Vulkan API version
        );

        m_instance = vk::createInstance(
            vk::InstanceCreateInfo(
                vk::InstanceCreateFlags { 0u },
                &appinfo,
                vulkan_validation_layers,
                vulkan_instance_extensions
            )
        );

        setupDebugMessenger(m_instance);
    }

    std::shared_ptr<Device> VkAdapterObject::createDeviceAPI()
    {
        return std::make_shared<VkDeviceObject>(
            getInstance()
        );
    }

    vk::Instance& VkAdapterObject::getInstance()
    {
        return m_instance;
    }
}