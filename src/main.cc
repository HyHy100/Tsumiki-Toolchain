#include <iostream>

#include <kate/adapter.h>
#include <kate/device.h>

int main(int argc, char* argv[]) {
    auto adapter = kate::gpu::Adapter::MakeVk();

    auto device = adapter->createDevice();

    //device->createSwapchain();

    std::cout << "hello!" << std::endl;

    return 0;
}