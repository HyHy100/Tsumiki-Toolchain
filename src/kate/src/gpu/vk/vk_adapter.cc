#include "vk_adapter.h"
#include "vk_device.h"
#include "vk_config.h"

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

        m_instance = vk::createInstance(
            vk::InstanceCreateInfo(
                vk::InstanceCreateFlags { 0u },
                &appinfo,
                vulkan_validation_layers,
                vulkan_instance_extensions
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