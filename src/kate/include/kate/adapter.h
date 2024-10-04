#pragma once

#include <memory>

namespace kate::gpu {
    class Device;

    class Adapter {
    public:
        std::shared_ptr<Device> createDevice();

        virtual ~Adapter() = default;
    protected:
        virtual std::shared_ptr<Device> createDeviceAPI() = 0; 
    };
}