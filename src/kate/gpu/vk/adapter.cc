#include "adapter.h"
#include "device.h"

namespace kate::gpu {
    VkAdapterObject::VkAdapterObject()
        : m_instance { vk::createInstance({}) }
    {
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