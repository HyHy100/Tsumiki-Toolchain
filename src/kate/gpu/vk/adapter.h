#pragma once

#include "kate/gpu/adapter.h"
#include "kate/gpu/device.h"

#include <memory>

#include <vulkan/vulkan.hpp>

namespace kate::gpu {
    class VkAdapterObject : public Adapter, public std::enable_shared_from_this<VkAdapterObject> {
    public:
        VkAdapterObject();

        vk::Instance& getInstance();
    private:
        std::shared_ptr<Device> createDeviceAPI() override final;

        vk::Instance m_instance;
    };
}