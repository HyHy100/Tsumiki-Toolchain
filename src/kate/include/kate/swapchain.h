#pragma once

#include "base/flags.h"

#include "texture.h"

#include <vector>

namespace kate::gpu {
    struct PlatformHandle {
        // X server
        uint32_t xcb_window;
        void* xcb_connection;

        // TODO: Wayland
        // TODO: Windows
    };

    enum class SwapchainFlagsBits {
        kNone,
        kVSync
    };

    using SwapchainFlags = base::Flags<SwapchainFlagsBits>;

    class Swapchain {
    public:
        virtual ~Swapchain() = default;
    private:
        std::vector<Texture> m_textures;
    };
}