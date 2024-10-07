#pragma once

#include <cstddef>

namespace kate::gpu {
  class Memory {
  public:
    Memory(
      void* ptr,
      size_t size
    );

    void* data();

    size_t size();
  private:
    void* m_ptr;
    size_t m_size;
  };
}