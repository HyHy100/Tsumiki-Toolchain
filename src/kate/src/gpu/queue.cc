#include "queue.h"

namespace kate::gpu {
    Queue::Queue(QueueFlagBits flags)
        : m_flags { flags }
    {
    }

    QueueFlags Queue::flags() const
    {
        return m_flags;
    }
}