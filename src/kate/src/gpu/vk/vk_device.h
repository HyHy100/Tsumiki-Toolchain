#pragma once

#include "kate/device.h"
#include "kate/extent.h"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace kate::gpu {
    class VkAdapterObject;
    class VkSwapChainObject;
    class VkQueueObject;

    class VkDeviceObject : public Device, public std::enable_shared_from_this<VkDeviceObject> {
    public:
        VkDeviceObject(std::shared_ptr<VkAdapterObject> adapter);

        vk::Device& getDevice();

        vk::PhysicalDevice& getPhysicalDevice();

        std::shared_ptr<VkAdapterObject> getAdapter();

        std::shared_ptr<Texture> createTexture(
            Texture::Dimension dimension,
            Texture::Format format,
            const Extent& extent,
            uint16_t layers
        ) override;

        std::shared_ptr<Swapchain> createSwapchain(
            const PlatformHandle& handle,
            uint16_t width,
            uint16_t height,
            const SwapchainFlags& flags
        ) override;

        uint32_t getMemoryTypeIndex(uint32_t typeBits, vk::MemoryPropertyFlags properties);

        size_t numQueues() const override;

        std::shared_ptr<Queue> getQueue(size_t index) override;

        std::shared_ptr<Queue> getQueue(QueueFlags index) override;
    private:
        friend class VkSwapChainObject;

        void setPresentationQueue(
            std::shared_ptr<VkQueueObject> queue
        );

        std::shared_ptr<VkAdapterObject> m_adapter;
        std::vector<std::shared_ptr<Queue>> m_queues;
        vk::Device m_device;
        vk::PhysicalDevice m_physicalDevice;
    };
}