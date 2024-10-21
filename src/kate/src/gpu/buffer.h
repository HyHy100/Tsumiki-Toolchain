#pragma once

#include <cstdint>

namespace kate::gpu {
  class Buffer {
  public:
    struct Flags {
      enum Enum {
        kHostVisible,
        kDeviceVisible
      };
    };

    Buffer(uint32_t size, Flags::Enum flags);

    uint32_t size() const;

    Flags::Enum flags() const;
  private:
    uint32_t m_size;
    Flags::Enum m_flags;
  };
}