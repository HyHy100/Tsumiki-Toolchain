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
            uint32_t queueFamilyIndex,
            uint32_t queueIndex
        );

        uint32_t queueFamilyIndex() const;

        uint32_t queueIndex() const;
    private:
        std::shared_ptr<VkDeviceObject> m_device;
        vk::Queue m_queue;
        uint32_t m_queueFamilyIndex;
        uint32_t m_queueIndex;
    };
}