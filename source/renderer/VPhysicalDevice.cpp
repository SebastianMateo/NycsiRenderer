#include "VPhysicalDevice.hpp"

#include <iostream>
#include <set>
#include <vector>

namespace Renderer::VPhysicalDevice
{
    VkPhysicalDevice CreatePhysicalDevice(const VkInstance vkInstance, const VkSurfaceKHR vkSurface)
    {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        
        // Query for available devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

        // If can't find any, there's no need to continue
        if (deviceCount == 0)
        {   
            std::cout << "ERROR: Failed to find GPUs with Vulkan support!!" << '\n';
            return VK_NULL_HANDLE;
        }

        // Get a vector with all physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

        // Iterate all devices for one suitable
        for (const auto& device : devices)
        {
            if (IsDeviceSuitable(device, vkSurface))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            std::cout << "failed to find a suitable GPU!" << '\n';
        }

        return physicalDevice;
    }

    bool IsDeviceSuitable(VkPhysicalDevice vkPhysicalDevice, const VkSurfaceKHR vkSurface)
    {
        // We check if we have a queue family that works for us
        const QueueFamilyIndices indices = FindQueueFamilies(vkPhysicalDevice, vkSurface);

        // Check if we support the extensions we need
        const bool extensionsSupported = CheckDeviceExtensionSupport(vkPhysicalDevice);

        // And then, if we have the correct swapChain support
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            const Renderer::SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(vkPhysicalDevice, vkSurface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &supportedFeatures);
        
        return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice vkPhysicalDevice, const VkSurfaceKHR vkSurface)
    {
        QueueFamilyIndices indices;

        // Get the queue families we have in our physical device
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

        // Check for the capabilities that we need
        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            // Render capabilities
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            // And if we have present capabilities
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
            
            // Early exit. We could end with two queues
            if (indices.IsComplete()) break;
    
            i++;
        }

        return indices;
    }

    bool CheckDeviceExtensionSupport(const VkPhysicalDevice device)
    {
        // Get the available extensions for this device
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // And we verify we have what we want
        std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

        for (const VkExtensionProperties& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR vkSurface)
    {
        SwapChainSupportDetails details;
        
        // Takes the specified VkPhysicalDevice and VkSurfaceKHR window surface into account when
        // determining the supported capabilities as they are the core components of the swap chain
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &details.capabilities);

        // Query the supported surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, details.formats.data());
        }

        // Query the supported presentation modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, details.presentModes.data());
        }

        // Now we have everything on details, we can return it
        return details;
    }

    VkSampleCountFlagBits GetMaxUsableSampleCount(const VkPhysicalDevice vkPhysicalDevice)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);

        const VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
            physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    uint32_t FindMemoryType(const VkPhysicalDevice vkPhysicalDevice, const uint32_t typeFilter, const VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    VkFormat FindSupportedFormat(
        const VkPhysicalDevice vkPhysicalDevice,
        const std::vector<VkFormat>& candidates,
        const VkImageTiling tiling,
        const VkFormatFeatureFlags features)
    {
        for (const VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;

            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat FindDepthFormat(const VkPhysicalDevice vkPhysicalDevice)
    {
        return VPhysicalDevice::FindSupportedFormat(
            vkPhysicalDevice,
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    };
}
