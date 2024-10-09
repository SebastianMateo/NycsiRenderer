#pragma once
#include "vulkan/vulkan_core.h"

namespace Renderer::VSwapChain { struct VSwapChain; }

namespace Renderer::VLogicalDevice
{
    VkDevice CreateLogicalDevice(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);

    VkRenderPass CreateRenderPass(
        VkPhysicalDevice vkPhysicalDevice,
        VkDevice vkDevice,
        VSwapChain::VSwapChain& swapChain,
        VkSampleCountFlagBits msaaSamples
    );
}
