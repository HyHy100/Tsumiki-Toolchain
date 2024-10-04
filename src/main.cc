#include <iostream>

#include <kate/adapter.h>

int main(int argc, char* argv[]) {
    auto adapter = kate::gpu::Adapter::MakeVk();

    auto device = adapter->createDevice();

    std::cout << "hello!" << std::endl;

    return 0;
}