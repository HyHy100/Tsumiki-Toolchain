#pragma once

#include <memory>

namespace kate::gpu {
  class Device;

  class Adapter {
  public:
    virtual std::shared_ptr<Device> createDevice() = 0;

    virtual ~Adapter() = default;
    
    static std::shared_ptr<Adapter> MakeVk();
  };
}