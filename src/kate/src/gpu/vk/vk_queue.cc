#include "vk_queue.h"

namespace kate::gpu {
    VkQueueObject::VkQueueObject(
        const QueueFlags& flags,
        std::shared_ptr<VkDeviceObject> device,
        uint32_t queueFamilyIndex,
        uint32_t queueIndex
    ) : Queue(flags), 
        m_device { device }, 
        m_queue { 
            device->getDevice().getQueue(
                queueFamilyIndex, 
                queueIndex
            )
        },
        m_queueIndex { queueIndex },
        m_queueFamilyIndex { queueFamilyIndex }
    {
    }

    uint32_t VkQueueObject::queueFamilyIndex() const
    {
        return m_queueFamilyIndex;
    }

    uint32_t VkQueueObject::queueIndex() const
    {
        return m_queueIndex;
    }
}