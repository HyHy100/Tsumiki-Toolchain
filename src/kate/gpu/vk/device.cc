#include "device.h"

#include "adapter.h"

namespace kate::gpu {
    VkDeviceObject::VkDeviceObject(
        std::shared_ptr<VkAdapterObject> adapter
    ) : m_adapter { adapter }
    {
        auto& vk_instance = adapter->getInstance();

        auto physical_devices = vk_instance.enumeratePhysicalDevices();

        // TODO: Pick a physical device properly.
        vk::PhysicalDevice physical_device = physical_devices.at(0);

        m_device = physical_device.createDevice({});
    }

    vk::Device& VkDeviceObject::getDevice()
    {
        return m_device;
    }
}