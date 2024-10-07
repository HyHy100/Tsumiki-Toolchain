#include "extent.h"

namespace kate::gpu {
  Extent::Extent() : 
    m_width { 0u },
    m_height { 0u },
    m_depth { 0u }
  {
  }

  Extent::Extent(
    uint16_t width,
    uint16_t height,
    uint16_t depth
  ) : m_width { width },
    m_height { height },
    m_depth { depth }
  {
  }

  uint16_t Extent::width() const
  {
    return m_width;
  }

  uint16_t Extent::height() const
  {
    return m_height;
  }

  uint16_t Extent::depth() const
  {
    return m_depth;
  }
}