#include "vk_texture.h"
#include "vk_device.h"

namespace kate::gpu {
  VkTextureObject::VkTextureObject(
    std::shared_ptr<VkDeviceObject> device,
    Texture::Dimension dimension,
    Texture::Format format,
    const Extent& extent,
    uint16_t layers
  ) : m_device { device }
  {
    auto vk_device = device->getDevice();

    vk::ImageType vktype;
    vk::ImageViewType vk_view_type;

    switch (dimension) {
      case Texture::Dimension::k1D:
        vktype = vk::ImageType::e1D;
        vk_view_type = vk::ImageViewType::e1D;
        break;
      case Texture::Dimension::k2D:
        vktype = vk::ImageType::e2D;
        vk_view_type = vk::ImageViewType::e2D;
        break;
      case Texture::Dimension::k3D:
        vktype = vk::ImageType::e3D;
        vk_view_type = vk::ImageViewType::e3D;
        break;
    }

    vk::Format vk_format;
    vk::ImageAspectFlags aspect;

    switch (format) {
      case Texture::Format::kRGBA8:
        vk_format = vk::Format::eR8G8B8A8Unorm;
        aspect = vk::ImageAspectFlagBits::eColor;
        break;
      case Texture::Format::kD24S8:
        vk_format = vk::Format::eD24UnormS8Uint;
        aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        break;
      case Texture::Format::kBGRA8:
        vk_format = vk::Format::eB8G8R8A8Unorm;
        aspect = vk::ImageAspectFlagBits::eColor;
        break;
    }

    vk::ImageUsageFlagBits vk_usage;
    
    switch (format) {
      case Texture::Format::kD24S8:
        vk_usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        break;
      default:
        vk_usage = vk::ImageUsageFlagBits::eColorAttachment;
        break;
    }

    auto vk_image_create_info = vk::ImageCreateInfo(
      {},
      vktype,             // type
      vk_format,            // format
      vk::Extent3D {          // extent
        extent.width(),
        extent.height(),
        extent.depth()
      },
      0,                // mip level
      layers,             // array layers
      vk::SampleCountFlagBits::e1,  // sample count
      vk::ImageTiling::eOptimal,    // image tiling
      vk_usage,             // usage
      vk::SharingMode::eExclusive,  // sharing mode
      {}                // TODO (renan): Fix this once we have a basic build.
    );

    m_image = vk_device.createImage(vk_image_create_info);

    m_imageView = vk_device.createImageView(
      vk::ImageViewCreateInfo(
        vk::ImageViewCreateFlags { 0u },// empty flags
        m_image,             // image
        vk_view_type,           // view type
        vk_format,            // view format 
        vk::ComponentMapping(),     // swizzle
        vk::ImageSubresourceRange(
          aspect,           // Aspect mask
          0,              // Base mip level
          1,              // Mip level
          0,              // Base array layer
          layers            // Layer count
        )
      )
    );

    vk::ImageAspectFlagBits a = static_cast<vk::ImageAspectFlagBits>(aspect.operator VkImageCreateFlags());

    auto memory_requirements = vk_device.getImageMemoryRequirements(m_image);

    m_memory = vk_device.allocateMemory(
      vk::MemoryAllocateInfo(
        memory_requirements.size,
        m_device->getMemoryTypeIndex(
          memory_requirements.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal
        )
      )
    );
  }
}