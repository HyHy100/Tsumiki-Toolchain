#pragma once

#include "kate/device.h"
#include "kate/extent.h"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace kate::gpu {
    class VkAdapterObject;

    class VkDeviceObject : public Device, public std::enable_shared_from_this<VkDeviceObject> {
    public:
        VkDeviceObject(std::shared_ptr<VkAdapterObject> adapter);

        vk::Device& getDevice();

        std::shared_ptr<Texture> createTexture(
            Texture::Dimension dimension,
            Texture::Format format,
            const Extent& extent,
            uint16_t layers
        ) override;
    private:
        std::shared_ptr<VkAdapterObject> m_adapter;

        vk::Device m_device;
    };
}