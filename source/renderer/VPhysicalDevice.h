#pragma once
#include <optional>
#include <vector>

#include "vulkan/vulkan_core.h"

const std::vector DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// Possible that graphic and supporting family do not overlap
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VPhysicalDevice
{
public:
    explicit VPhysicalDevice(VkInstance vkInstance, VkSurfaceKHR vkSurface);
    VPhysicalDevice() = default;

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device = VK_NULL_HANDLE) const;
    
    // We need to know
    //  * Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
    //  * Surface formats (pixel format, color space)
    //  * Available presentation modes
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device = VK_NULL_HANDLE) const;

    VkSampleCountFlagBits GetMaxUsableSampleCount() const;
    
    VkPhysicalDevice Get() const { return mVkPhysicalDevice; }

private:
    bool IsDeviceSuitable(VkPhysicalDevice_T* device) const;
    
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    
    VkPhysicalDevice mVkPhysicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR mVkSurface = VK_NULL_HANDLE;
};
