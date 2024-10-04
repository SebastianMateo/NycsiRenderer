#pragma once
#include <vector>

#include "VPods.hpp"
#include "vulkan/vulkan_core.h"

namespace Renderer::VPhysicalDevice
{
    const std::vector DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkPhysicalDevice CreatePhysicalDevice(VkInstance vkInstance, VkSurfaceKHR vkSurface);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);
    
    // We need to know
    //  * Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
    //  * Surface formats (pixel format, color space)
    //  * Available presentation modes
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);

    VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice vkPhysicalDevice);

    bool IsDeviceSuitable(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice);
}