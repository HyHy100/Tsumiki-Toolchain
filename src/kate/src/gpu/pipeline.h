#pragma once

#include <span>
#include <cstdint>
#include <vector>
#include <variant>
#include "buffer.h"

namespace kate::gpu {
  class Resource {
  public:
    Resource() = delete;

    Resource(const Buffer& buffer);

    enum class Type {
      kBuffer
    };

    Type type() const;
  private:
    Type m_type;
    
    union {
      Buffer buffer;
    } m_obj;
  };
  
  class Pipeline {
  public:
    class Binding {
    public:
      Binding() = delete;

      Binding(
        uint32_t group, 
        uint32_t index,
        Resource resource
      );

      uint32_t group() const;

      uint32_t index() const;


    private:
      uint32_t m_index;
      uint32_t m_group;
    };

    class Layout {
    public:
      Layout(
        const std::span<Binding>& bindings
      );

      const std::vector<Binding>& bindings();
    private:
      const std::vector<Binding> m_bindings;
    };

    Pipeline(
      const Layout& layout
    );
  };
}