#include "vk_swapchain.h"
#include "vk_adapter.h"
#include "vk_device.h"
#include "vk_config.h"
#include "vk_queue.h"

#ifdef __linux
#   include <X11/Xlib.h>
#   include <vulkan/vulkan_xlib.h>
#endif

#include <limits>

namespace kate::gpu {
  VkSwapChainObject::VkSwapChainObject(
    std::shared_ptr<VkDeviceObject> device,
    const PlatformHandle& handle,
    uint16_t width,
    uint16_t height,
    const SwapchainFlags& flags
  ) : m_device { device }
  {
    auto& instance = m_device->getAdapter()->getInstance();

#     ifdef __linux
    VkXlibSurfaceCreateInfoKHR create_info = {};
    create_info.flags = 0;
    create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.window = reinterpret_cast<XID>(handle.nwd);
    create_info.dpy = reinterpret_cast<Display*>(handle.ndt);
    vkCreateXlibSurfaceKHR(
      instance,
      &create_info,
      nullptr,
      // Renan: it is unlikely, but maybe this cast could cause issues in the future.
      reinterpret_cast<VkSurfaceKHR*>(&m_surface)
    );
#     else
#       error "Platform is not supported yet."
#     endif

    auto& physical_device = m_device->getPhysicalDevice();

    auto props = physical_device.getQueueFamilyProperties();

    std::vector<VkBool32> isPresentationSupported(props.size());

    for (auto i = 0; i < props.size(); i++) {
      auto& prop = props[i];

      isPresentationSupported[i] = physical_device.getSurfaceSupportKHR(i, m_surface);
    }

    uint32_t present_queue_family_index = 
      std::numeric_limits<uint32_t>::max();

    for (auto i = 0; i < props.size(); i++) {
      auto& prop = props[i];

      if (isPresentationSupported[i]) {
        present_queue_family_index = i;

        break;
      }
    }

    if (present_queue_family_index == 
        std::numeric_limits<uint32_t>::max()) {
      // abort
      throw "(Vulkan): Unable to find a vulkan queue that supports presentation.";
    }

    device->setPresentationQueue(
      std::make_shared<VkQueueObject>(
        QueueFlagBits::kPresentation,
        device,
        present_queue_family_index,
        0
      )
    );

    auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(
      m_surface
    );

    VkExtent2D swapchainExtent;

    if (surface_capabilities.currentExtent.width == 
      std::numeric_limits<uint32_t>::max()) {
      swapchainExtent.width = std::clamp(
        static_cast<uint32_t>(width), 
        surface_capabilities.minImageExtent.width, 
        surface_capabilities.maxImageExtent.width
      );

      swapchainExtent.height = std::clamp(
        static_cast<uint32_t>(height), 
        surface_capabilities.minImageExtent.height, 
        surface_capabilities.maxImageExtent.height
      );
    } else {
      swapchainExtent = surface_capabilities.currentExtent;
    }

    vk::SurfaceTransformFlagBitsKHR pre_transform;
    if (surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
      pre_transform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    else 
      pre_transform = surface_capabilities.currentTransform;

    vk::CompositeAlphaFlagBitsKHR composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

    std::array<vk::CompositeAlphaFlagBitsKHR, 4> composite_alpha_flags {
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
      vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
      vk::CompositeAlphaFlagBitsKHR::eInherit
    };

    for (auto& composite_alpha_flag : composite_alpha_flags)
      if (surface_capabilities.supportedCompositeAlpha & composite_alpha_flag) {
        composite_alpha = composite_alpha_flag;
        break;
      }

    auto surface_formats = m_device->getPhysicalDevice().getSurfaceFormatsKHR(m_surface);

    if (surface_formats.empty())
      throw "no surface formats available, please check your GPU drivers.";

    auto* vk_graphics_queue = static_cast<VkQueueObject*>(
      m_device->getQueue(QueueFlagBits::kGraphics).get()
    );

    auto* vk_presentation_queue = static_cast<VkQueueObject*>(
      m_device->getQueue(QueueFlagBits::kPresentation).get()
    );

    std::array<uint32_t, 2> queueFamilyIndices = {
      vk_graphics_queue->queueFamilyIndex(),
      vk_presentation_queue->queueFamilyIndex()
    };

    vk::SwapchainCreateInfoKHR swapchain_ci( 
      vk::SwapchainCreateFlagsKHR {},
      m_surface,
      std::clamp(
        3u, 
        surface_capabilities.minImageCount, 
        surface_capabilities.maxImageCount
      ),
      vk::Format::eB8G8R8A8Unorm,
      vk::ColorSpaceKHR::eSrgbNonlinear,
      swapchainExtent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eConcurrent,
      queueFamilyIndices,
      pre_transform,
      composite_alpha,
      kVkDefaultPresentationMode,
      true,
      nullptr
    );

    m_swapchain = m_device->getDevice().createSwapchainKHR(swapchain_ci);

    m_swapchainImages = m_device->getDevice().getSwapchainImagesKHR(m_swapchain);

    m_swapchainImageViews.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
      m_swapchainImageViews[i] = m_device->getDevice().createImageView(
        vk::ImageViewCreateInfo(
          vk::ImageViewCreateFlags {},
          m_swapchainImages[i],
          vk::ImageViewType::e2D,
          vk::Format::eB8G8R8A8Unorm,
          vk::ComponentMapping {
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity
          },
          vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor,
            0, // base mip level
            1, // mip level
            0, // base array layer
            1  // layer count
          )
        )
      );
    }
  }
}