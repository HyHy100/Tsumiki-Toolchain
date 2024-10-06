#include "vk_adapter.h"
#include "vk_device.h"
#include "vk_config.h"

#include <fmt/format.h>

namespace kate::gpu {
    VkDebugUtilsMessengerEXT debugMessenger;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        vk::DebugUtilsMessageTypeFlagsEXT messageType, 
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData
    ) {
        if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            fmt::println("『Vulkan』: ERROR: {}", pCallbackData->pMessage);

        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo(
        vk::DebugUtilsMessengerCreateInfoEXT& createInfo
    ) {
        createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
            {},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose 
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning 
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral 
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation 
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback)
        );
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ) {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(
                instance, 
                "vkCreateDebugUtilsMessengerEXT"
            )
        );
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void setupDebugMessenger(vk::Instance& instance) {
#       ifndef NDEBUG
        vk::DebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(
                instance, 
                reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), 
                nullptr,
                reinterpret_cast<VkDebugUtilsMessengerEXT*>(&debugMessenger)
            ) != VK_SUCCESS) 
            throw std::runtime_error("failed to set up debug messenger!");
#       endif
    }

    VkAdapterObject::VkAdapterObject()
    {
        vk::ApplicationInfo appinfo = vk::ApplicationInfo(
            "",                             // Application name
            VK_MAKE_VERSION(1, 0, 0),       // Application version
            "kate",                         // Engine name
            VK_MAKE_VERSION(0, 1, 0),       // Engine version
            VK_MAKE_API_VERSION(1, 0, 0, 0)  // Vulkan API version
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

    std::shared_ptr<Device> VkAdapterObject::createDevice()
    {
        auto device = std::make_shared<VkDeviceObject>(shared_from_this());

        device->initialize();

        return device;
    }

    vk::Instance& VkAdapterObject::getInstance()
    {
        return m_instance;
    }

    std::shared_ptr<Adapter> Adapter::MakeVk()
    {
        return std::make_shared<VkAdapterObject>();
    }
}