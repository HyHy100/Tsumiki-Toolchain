#pragma once

#include "compute_encoder.h"
#include "render_encoder.h"

#include <base/flags.h>

#include <memory>

namespace kate::gpu {
    enum class QueueFlagBits {
        kCompute = 0x1,
        kGraphics = 0x2,
        kTransfer = 0x4,
        kPresentation = 0x8
    };

    using QueueFlags = base::Flags<QueueFlagBits>;

    class Queue {
    public:
        Queue(QueueFlagBits flags);

        virtual ~Queue() = default;

        QueueFlags flags() const;

        virtual std::shared_ptr<ComputeEncoder> createComputeEncoder() { return nullptr; }

        virtual std::shared_ptr<RenderEncoder> createRenderEncoder() {  return nullptr; }

        virtual void submit() { /**/ }
    private:
        QueueFlags m_flags;
    };
}