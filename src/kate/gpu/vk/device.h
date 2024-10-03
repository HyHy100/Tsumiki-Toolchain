#pragma once

#include "kate/gpu/device.h"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace kate::gpu {
    class VkAdapterObject;

    class VkDeviceObject : public Device, public std::enable_shared_from_this<VkDeviceObject> {
    public:
        VkDeviceObject(std::shared_ptr<VkAdapterObject> adapter);

        vk::Device& getDevice();
    private:
        std::shared_ptr<VkAdapterObject> m_adapter;

        vk::Device m_device;
    };
}