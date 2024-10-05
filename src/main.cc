#include <iostream>
#include <thread>

#include <kate/adapter.h>
#include <kate/device.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#ifdef __linux
#   include <X11/Xlib.h>
#endif

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    auto* window = SDL_CreateWindow("hello!", 200, 200, SDL_WINDOW_RESIZABLE);

    void* ndt = nullptr;
    void* nwd = nullptr;

    #ifdef __linux
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
        ndt = (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        nwd = (void*)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (!ndt || !nwd) {
            std::cerr << "Cannot create XServer window." << std::endl;
            return 1;
        }
    } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        std::cerr << "Wayland not supported." << std::endl;
        return 1; 
    }
    #endif

    if (!window)
        throw "(SDL3): Failed to create window.";

    auto adapter = kate::gpu::Adapter::MakeVk();

    auto device = adapter->createDevice();

    kate::gpu::PlatformHandle handle;
    handle.nwd = nwd;
    handle.ndt = ndt;

    auto swapchain = device->createSwapchain(
        handle,
        200,
        200,
        kate::gpu::SwapchainFlagsBits::kVSync
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    SDL_Quit();

    return 0;
}