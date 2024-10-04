#include "vk_adapter.h"
#include "vk_device.h"

namespace kate::gpu {
    VkAdapterObject::VkAdapterObject()
    {
        vk::ApplicationInfo appinfo = vk::ApplicationInfo(
            "",                             // Application name
            vk::makeVersion(1, 0, 0),       // Application version
            "kate",                         // Engine name
            vk::makeVersion(0, 1, 0),       // Engine version
            vk::makeApiVersion(1, 0, 0, 0)  // Vulkan API version
        );
        
#       ifndef NDEBUG
        std::array<const char* const, 1> layers {
            "VK_LAYER_KHRONOS_validation"
        };
#       else
        std::array<const char* const, 0> layers {};
#       endif

        m_instance = vk::createInstance(
            vk::InstanceCreateInfo(
                vk::InstanceCreateFlags { 0u },
                &appinfo,
                {},
                layers
            )
        );
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