#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "queue.h"
#include "vk_device.h"

namespace kate::gpu {
    class VkQueueObject : public std::enable_shared_from_this<VkQueueObject>, public Queue {
    public:
        VkQueueObject(
            const QueueFlags& flags,
            std::shared_ptr<VkDeviceObject> device,
            vk::Queue queue
        );
    private:
        std::shared_ptr<VkDeviceObject> m_device;
        vk::Queue m_queue;
    };
}