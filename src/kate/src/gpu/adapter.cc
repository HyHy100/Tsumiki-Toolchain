#include "adapter.h"

namespace kate::gpu {
    std::shared_ptr<Device> Adapter::createDevice()
    {
        return createDeviceAPI();
    }
}