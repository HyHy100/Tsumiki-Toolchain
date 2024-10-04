#include "vk_adapter.h"
#include "vk_config.h"
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

        auto queue_family_properties = physical_device.getQueueFamilyProperties();

        bool has_dedicated_compute_queue = false;
        bool has_dedicated_transfer_queue = false;
        bool has_dedicated_draw_queue = false;

        uint32_t compute_queue_index,
                draw_queue_index,
                transfer_queue_index;

        for (size_t i = 0; i < queue_family_properties.size(); i++) {
            // If there is a dedicated queue, give preference to it.
            if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eCompute && !has_dedicated_compute_queue) {
                if (!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
                    has_dedicated_compute_queue = true;
                    compute_queue_index = i;
                } else {
                    compute_queue_index = i;
                }
            }

            if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics && !has_dedicated_draw_queue) {
                if (!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eCompute)) {
                    has_dedicated_draw_queue = true;
                    draw_queue_index = i;
                } else {
                    draw_queue_index = i;
                }
            }

            if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eTransfer && !has_dedicated_transfer_queue) {
                if (!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
                    has_dedicated_transfer_queue = true;
                    transfer_queue_index = i;
                } else {
                    transfer_queue_index = i;
                }
            }

            if (has_dedicated_compute_queue && has_dedicated_draw_queue && has_dedicated_transfer_queue)
                break;
        }

        auto queue_create_infos = std::array {
            vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags { 0u },
                draw_queue_index,
                1
            ),
            vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags { 0u },
                compute_queue_index,
                1
            ),
            vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags { 0u },
                transfer_queue_index,
                1
            ),
        };

        m_device = physical_device.createDevice(
            vk::DeviceCreateInfo(
                vk::DeviceCreateFlags { 0u },
                queue_create_infos.size(),      // Queue create info count.
                queue_create_infos.data(),      // Queue create pointer.
                vulkan_validation_layers.size(),// Device layer count.
                &vulkan_validation_layers[0],   // Device layer pointer.
                vulkan_device_extensions.size(),// Extension count.
                &vulkan_device_extensions[0],   // Extension list.
                nullptr,                        // Enabled features.
                nullptr                         // pNext.
            )
        );
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