#pragma once
#include <vector>

#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

namespace Renderer::VSwapChain
{
    struct VSwapChain
    {
        VkSwapchainKHR vkSwapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D swapChainExtent = {};
    };
        
    VSwapChain CreateSwapChain(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface, VkDevice vkDevice, GLFWwindow* glfwWindow);
};
