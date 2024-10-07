#pragma once

#include "extent.h"
#include "texture.h"
#include "swapchain.h"
#include "sampler.h"
#include "queue.h"

#include "base/flags.h"

#include <memory>
#include <bit>
#include <type_traits>

namespace kate::gpu {
  enum class StateFlagBits : uint64_t {
    kRGB = 0x1,
    kA = 0x2,
    kDepth = 0x4,
    kStencil = 0x8,

    kWriteMaskBits = kRGB | kA | kDepth | kStencil,
    
    // address modes
    kAddressModeUClampToEdge = 0x10,
    kAddressModeURepeat = 0x20,
    kAddressModeUMirrorRepeat = 0x40,

    kAddressMoveUMaskBits = kAddressModeUClampToEdge | kAddressModeURepeat | kAddressModeUMirrorRepeat,

    kAddressModeVClampToEdge = 0x80,
    kAddressModeVRepeat = 0x100,
    kAddressModeVMirrorRepeat = 0x200,

    kAddressMoveVMaskBits = kAddressModeVClampToEdge | kAddressModeVRepeat | kAddressModeVMirrorRepeat,

    kAddressModeClampToEdge = kAddressModeUClampToEdge | kAddressModeVClampToEdge,
    kAddressModeRepeat = kAddressModeURepeat | kAddressModeVRepeat,
    kAddressModeMirrorRepeat = kAddressModeUMirrorRepeat | kAddressModeVMirrorRepeat,

    kCompareNever = 0x400,
    kCompareLess = 0x800,
    kCompareEqual = 0x1000,
    kCompareLessEqual = 0x2000,
    kCompareGreater = 0x4000,
    kCompareNotEqual = 0x8000,
    kCompareGreaterEqual = 0x10000,
    kCompareAlways = 0x20000,

    kCompareMaskBits =  kCompareNever | 
              kCompareLess | 
              kCompareEqual | 
              kCompareLessEqual | 
              kCompareGreater | 
              kCompareNotEqual | 
              kCompareGreaterEqual | 
              kCompareAlways,

    kMagFilterNearest = 0x40000,
    kMagFilterLinear = 0x80000,

    kMagFilterMaskBits = kMagFilterNearest | kMagFilterLinear,

    kMinFilterNearest = 0x100000,
    kMinFilterLinear = 0x200000,

    kMinFilterMaskBits = kMinFilterNearest | kMinFilterLinear,

    kMipmapFilterNearest = 0x400000,
    kMipmapFilterLinear = 0x800000,

    kMipmapFilterMaskBits = kMipmapFilterNearest | kMipmapFilterLinear
  };

  using StateFlags = base::Flags<StateFlagBits>;

  class Device {
  public:
    virtual ~Device() = default;

    virtual std::shared_ptr<Texture> createTexture(
      Texture::Dimension dimension,
      Texture::Format format,
      const Extent& extent,
      uint16_t layers = 0
    ) = 0;

    virtual std::shared_ptr<Swapchain> createSwapchain(
      const PlatformHandle& handle,
      uint16_t width,
      uint16_t height,
      const SwapchainFlags& flags = SwapchainFlagsBits::kNone
    ) = 0;

    virtual size_t numQueues() const = 0;

    virtual std::shared_ptr<Queue> getQueue(size_t index) = 0;

    virtual std::shared_ptr<Queue> getQueue(QueueFlags flags) = 0;
  };
}