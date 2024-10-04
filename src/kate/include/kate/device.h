#pragma once

#include "extent.h"
#include "texture.h"
#include "swapchain.h"

#include <memory>

namespace kate::gpu {


    class Device {
    public:
        Device();

        virtual ~Device() = default;

        virtual std::shared_ptr<Texture> createTexture(
            Texture::Dimension dimension,
            Texture::Format format,
            const Extent& extent,
            uint16_t layers = 0
        ) = 0;

        virtual std::shared_ptr<Swapchain> createSwapchain(
            const PlatformHandle& handle,
            uint16_t width,
            uint16_t height,
            const SwapchainFlags& flags = SwapchainFlagsBits::kNone
        ) = 0;
    };
}