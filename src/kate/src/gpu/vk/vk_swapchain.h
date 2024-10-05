#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "kate/swapchain.h"

namespace kate::gpu {
    class VkDeviceObject;

    class VkSwapChainObject : public std::enable_shared_from_this<VkSwapChainObject>, public Swapchain {
    public:
        VkSwapChainObject(
            std::shared_ptr<VkDeviceObject> device,
            const PlatformHandle& handle,
            uint16_t width,
            uint16_t height,
            const SwapchainFlags& flags
        );
    private:
        std::shared_ptr<VkDeviceObject> m_device;
        std::vector<vk::Image> m_swapchainImages;
        std::vector<vk::ImageView> m_swapchainImageViews;
        vk::SwapchainKHR m_swapchain;
        vk::SurfaceKHR m_surface;
    };
}