#pragma once
#include <vector>

#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

namespace Renderer::VSwapChain
{
    struct VSwapChain
    {
        VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D swapChainExtent = {};
    };
        
    VSwapChain CreateSwapChain(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface, VkDevice vkDevice, GLFWwindow* glfwWindow);
    VkImageView CreateImageView(VkDevice vkDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    void CleanupSwapChain(VkDevice vkDevice, VkImageView vkColorImageView, VkImage vkColorImage,
                          VkDeviceMemory vkColorImageMemory, const VSwapChain& vSwapChain,
                          const std::vector<VkFramebuffer>& swapChainFramebuffers);
}