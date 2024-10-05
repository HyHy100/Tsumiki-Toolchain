#include "vk_queue.h"

namespace kate::gpu {
    VkQueueObject::VkQueueObject(
        const QueueFlags& flags,
        std::shared_ptr<VkDeviceObject> device,
        vk::Queue queue
    ) : Queue(flags), 
        m_device { device }, 
        m_queue { queue }
    {
    }
}