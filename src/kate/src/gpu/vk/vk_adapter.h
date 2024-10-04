#pragma once

#include "kate/adapter.h"
#include "kate/device.h"

#include <memory>

#include <vulkan/vulkan.hpp>

namespace kate::gpu {
    class VkAdapterObject : public Adapter, public std::enable_shared_from_this<VkAdapterObject> {
    public:
        VkAdapterObject();

        std::shared_ptr<Device> createDevice() override final;

        vk::Instance& getInstance();
    private:
        vk::Instance m_instance;
    };
}