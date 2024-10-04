#pragma once

#include <memory>
#include <span>

#include "memory.h"

namespace kate::gpu {
    class Texture : public std::enable_shared_from_this<Texture> {
    public:
        enum class Dimension {
            k1D,
            k2D,
            k3D
        };

        enum class Format {
            kRGBA8,
            kBGRA8,
            kD24S8,
            kCount
        };

        virtual ~Texture() = default;

        virtual void update(const Memory& memory) = 0;
    };
}