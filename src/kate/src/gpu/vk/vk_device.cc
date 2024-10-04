#include "vk_adapter.h"
#include "vk_device.h"

namespace kate::gpu {
    VkDeviceObject::VkDeviceObject(
        std::shared_ptr<VkAdapterObject> adapter
    ) : m_adapter { adapter }
    {
        auto& vk_instance = adapter->getInstance();

        auto physical_devices = vk_instance.enumeratePhysicalDevices();

        // TODO: Pick a physical device properly.
        vk::PhysicalDevice physical_device = physical_devices.at(0);

        m_device = physical_device.createDevice({});
    }

    vk::Device& VkDeviceObject::getDevice()
    {
        return m_device;
    }

    std::shared_ptr<Texture> VkDeviceObject::createTexture(
        Texture::Dimension dimension,
        Texture::Format format,
        const Extent& extent,
        uint16_t layers
    )
    {
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

        switch (format) {
            case Texture::Format::kRGBA8:
                vk_format = vk::Format::eR8G8B8A8Unorm;
                break;
            case Texture::Format::kD24S8:
                vk_format = vk::Format::eD24UnormS8Uint;
                break;
            case Texture::Format::kBGRA8:
                vk_format = vk::Format::eB8G8R8A8Unorm;
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

        auto vk_image = getDevice().createImage(
            vk::ImageCreateInfo(
                {},
                vktype,                         // type
                vk_format,                      // format
                vk::Extent3D {                  // extent
                    extent.width(),
                    extent.height(),
                    extent.depth()
                },
                0,                              // mip level
                layers,                         // array layers
                vk::SampleCountFlagBits::e1,    // sample count
                vk::ImageTiling::eOptimal,      // image tiling
                vk_usage,                       // usage
                vk::SharingMode::eExclusive,    // sharing mode
                {}                              // TODO (renan): Fix this once we have a basic build.
            )
        );

        auto image_view = getDevice().createImageView(
            vk::ImageViewCreateInfo(
                vk::ImageViewCreateFlags { 0u },// empty flags
                vk_image,                       // image
                vk_view_type,                   // view type
                vk_format,                      // view format 
                vk::ComponentMapping()          // swizzle
            )
        );
    }
}