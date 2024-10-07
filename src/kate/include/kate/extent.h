#pragma once

#include <cstdint>

namespace kate::gpu {
  class Extent {
  public:
    Extent();

    Extent(uint16_t width, uint16_t height, uint16_t depth = 1);

    uint16_t width() const;

    uint16_t height() const;

    uint16_t depth() const;
  private:
    uint16_t m_width, 
         m_height, 
         m_depth;
  };
}