#pragma once
#include <vector>
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

    std::vector<VkFramebuffer> CreateFramebuffers(
        VkDevice vkDevice,
        VkRenderPass vkRenderPass,
        const VSwapChain::VSwapChain& vSwapChain,
        VkImageView vkColorImageView,
        VkImageView vkDepthImageView
    );

    VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice vkDevice);
}
