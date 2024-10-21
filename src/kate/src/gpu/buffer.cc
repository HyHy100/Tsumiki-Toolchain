#include "buffer.h"

namespace kate::gpu {
  Buffer::Buffer(
    uint32_t size,
    Flags::Enum flags
  ) : m_size { size },
      m_flags { flags }
  {
  }

  uint32_t Buffer::size() const
  {
    return m_size;
  }

  Buffer::Flags::Enum Buffer::flags() const
  {
    return m_flags;
  }
}