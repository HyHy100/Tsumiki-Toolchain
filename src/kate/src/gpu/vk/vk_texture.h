#pragma once

#include "kate/texture.h"
#include "kate/extent.h"

#include <vulkan/vulkan.hpp>

namespace kate::gpu {
  class VkDeviceObject;

  class VkTextureObject : public Texture, public std::enable_shared_from_this<VkTextureObject> {
  public:
    VkTextureObject(
      std::shared_ptr<VkDeviceObject> device,
      Texture::Dimension dimension,
      Texture::Format format,
      const Extent& extent,
      uint16_t layers
    );
  private:
    std::shared_ptr<VkDeviceObject> m_device;
    vk::Image       m_image;
    vk::ImageView     m_imageView;
    vk::DeviceMemory  m_memory;
  };
}