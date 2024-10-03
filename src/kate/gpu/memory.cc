#include "memory.h"

namespace kate::gpu {
    Memory::Memory(void* ptr, size_t size) 
        : m_ptr { ptr }, m_size { size }
    {
    }

    void* Memory::data()
    {
        return m_ptr;
    }

    size_t Memory::size()
    {
        return m_size;
    }
}