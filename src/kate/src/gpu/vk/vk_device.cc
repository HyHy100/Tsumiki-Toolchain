#include "vk_adapter.h"
#include "vk_config.h"
#include "vk_device.h"
#include "vk_texture.h"
#include "vk_swapchain.h"
#include "vk_queue.h"

#include <limits>

namespace kate::gpu {
    VkDeviceObject::VkDeviceObject(
        std::shared_ptr<VkAdapterObject> adapter
    ) : m_adapter { adapter }
    {
        auto& vk_instance = adapter->getInstance();

        auto physical_devices = vk_instance.enumeratePhysicalDevices();

        // TODO: Pick a physical device properly.
        vk::PhysicalDevice physical_device = physical_devices.at(0);
        m_physicalDevice = physical_device;

        auto queue_family_properties = physical_device.getQueueFamilyProperties();

        bool has_dedicated_compute_queue = false;
        bool has_dedicated_transfer_queue = false;
        bool has_dedicated_draw_queue = false;

        uint32_t compute_queue_family_index = std::numeric_limits<uint32_t>::max(),
                    draw_queue_family_index = std::numeric_limits<uint32_t>::max(),
                transfer_queue_family_index = std::numeric_limits<uint32_t>::max();

        for (size_t i = 0; i < queue_family_properties.size(); i++) {
            // If there is a dedicated queue, give preference to it.
            if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eCompute && !has_dedicated_compute_queue) {
                if (!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
                    has_dedicated_compute_queue = true;
                    compute_queue_family_index = i;
                } else {
                    compute_queue_family_index = i;
                }
            }

            if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics && !has_dedicated_draw_queue) {
                if (!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eCompute)) {
                    has_dedicated_draw_queue = true;
                    draw_queue_family_index = i;
                } else {
                    draw_queue_family_index = i;
                }
            }

            if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eTransfer && !has_dedicated_transfer_queue) {
                if (!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
                    has_dedicated_transfer_queue = true;
                    transfer_queue_family_index = i;
                } else {
                    transfer_queue_family_index = i;
                }
            }

            if (has_dedicated_compute_queue && has_dedicated_draw_queue && has_dedicated_transfer_queue)
                break;
        }

        if (draw_queue_family_index == std::numeric_limits<uint32_t>::max()) 
            throw "[Vulkan] Error while trying to create a draw queue.";
        else if (transfer_queue_family_index == std::numeric_limits<uint32_t>::max())
            throw "[Vulkan] Error while trying to create a transfer queue.";
        else if (compute_queue_family_index == std::numeric_limits<uint32_t>::max())
            throw "[Vulkan] Error while trying to create a compute queue.";

        auto queue_create_infos = std::array {
            vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags { 0u },
                draw_queue_family_index,
                1
            ),
            vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags { 0u },
                compute_queue_family_index,
                1
            ),
            vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags { 0u },
                transfer_queue_family_index,
                1
            ),
        };

        m_device = physical_device.createDevice(
            vk::DeviceCreateInfo(
                vk::DeviceCreateFlags { 0u },
                queue_create_infos.size(),      // Queue create info count.
                queue_create_infos.data(),      // Queue create pointer.
                vulkan_validation_layers.size(),// Device layer count.
                vulkan_validation_layers.data(),// Device layer pointer.
                vulkan_device_extensions.size(),// Extension count.
                vulkan_device_extensions.data(),// Extension list.
                nullptr,                        // Enabled features.
                nullptr                         // pNext.
            )
        );

        /*m_queues.push_back(
            std::make_shared<VkQueueObject>(
                QueueFlagBits::kGraphics,
                shared_from_this(),
                m_device.getQueue(draw_queue_family_index, 0)
            )
        );

        m_queues.push_back(
            std::make_shared<VkQueueObject>(
                QueueFlagBits::kCompute,
                shared_from_this(),
                m_device.getQueue(compute_queue_family_index, 0)
            )
        );

        m_queues.push_back(
            std::make_shared<VkQueueObject>(
                QueueFlagBits::kTransfer,
                shared_from_this(),
                m_device.getQueue(transfer_queue_family_index, 0)
            )
        );*/
    }

    vk::Device& VkDeviceObject::getDevice()
    {
        return m_device;
    }

    vk::PhysicalDevice& VkDeviceObject::getPhysicalDevice()
    {
        return m_physicalDevice;
    }

    std::shared_ptr<Texture> VkDeviceObject::createTexture(
        Texture::Dimension dimension,
        Texture::Format format,
        const Extent& extent,
        uint16_t layers
    )
    {
        return std::make_shared<VkTextureObject>(
            shared_from_this(),
            dimension,
            format,
            extent,
            layers
        );
    }

    std::shared_ptr<Swapchain> VkDeviceObject::createSwapchain(
        const PlatformHandle& handle,
        uint16_t width,
        uint16_t height,
        const SwapchainFlags& flags
    )
    {
        return std::make_shared<VkSwapChainObject>(
            shared_from_this(),
            handle,
            width,
            height,
            flags
        );
    }

    size_t VkDeviceObject::numQueues() const
    {
        return m_queues.size();
    }

    std::shared_ptr<Queue> VkDeviceObject::getQueue(size_t index)
    {
        return (index < m_queues.size()) ? m_queues[index] : nullptr;
    }

    std::shared_ptr<Queue> VkDeviceObject::getQueue(QueueFlags flags)
    {
        for (auto& queue : m_queues) {
            if (queue->flags() & flags)
                return queue;
        }

        // (Renan): TODO: Make this path not reachable.
        return nullptr;
    }

    void VkDeviceObject::setPresentationQueue(
        uint32_t familyIndex,
        uint32_t queueIndex
    )
    {
        m_queues.push_back(
            std::make_shared<VkQueueObject>(
                QueueFlagBits::kPresentation,
                shared_from_this(),
                m_device.getQueue(familyIndex, queueIndex)
            )
        );
    }

    std::shared_ptr<VkAdapterObject> VkDeviceObject::getAdapter()
    {
        return m_adapter;
    }

    uint32_t VkDeviceObject::getMemoryTypeIndex(uint32_t typeBits, vk::MemoryPropertyFlags properties)
	{
        vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = m_physicalDevice.getMemoryProperties();
        // Iterate over all memory types available for the device used in this example
        for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((typeBits & 1) == 1)
            {
                if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }
            typeBits >>= 1;
        }

        throw "Could not find a suitable memory type!";
    }
}