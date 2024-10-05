#pragma once

#include "base/flags.h"

#include "texture.h"

#include <vector>

namespace kate::gpu {
    struct PlatformHandle {
        void* nwd;
        void* ndt;
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