#include "VPhysicalDevice.h"

#include <iostream>
#include <set>
#include <vector>

VPhysicalDevice::VPhysicalDevice(const VkInstance vkInstance, const VkSurfaceKHR vkSurface)
    : mVkSurface(vkSurface)
{
    // Query for available devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    // If can't find any, there's no need to continue
    if (deviceCount == 0)
    {   
        std::cout << "ERROR: Failed to find GPUs with Vulkan support!!" << '\n';
        return;
    }

    // Get a vector with all physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    // Iterate all devices for one suitable
    for (const auto& physicalDevice : devices)
    {
        if (IsDeviceSuitable(physicalDevice))
        {
            mVkPhysicalDevice = physicalDevice;
            return;
        }
    }

    if (mVkPhysicalDevice == VK_NULL_HANDLE)
    {
        std::cout << "failed to find a suitable GPU!" << '\n';
    }
}

bool VPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice_T* device) const
{
    // We check if we have a queue family that works for us
    const QueueFamilyIndices indices = FindQueueFamilies(device);

    // Check if we support the extensions we need
    const bool extensionsSupported = CheckDeviceExtensionSupport(device);

    // And then, if we have the correct swapChain support
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    
    return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices VPhysicalDevice::FindQueueFamilies(VkPhysicalDevice device) const
{
    if (device == VK_NULL_HANDLE)
        device = mVkPhysicalDevice;
    
    QueueFamilyIndices indices;

    // Get the queue families we have in our physical device
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mVkSurface, &presentSupport);
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

bool VPhysicalDevice::CheckDeviceExtensionSupport(const VkPhysicalDevice device)
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

SwapChainSupportDetails VPhysicalDevice::QuerySwapChainSupport(VkPhysicalDevice device) const
{
    if (device == VK_NULL_HANDLE)
        device = mVkPhysicalDevice;
    
    SwapChainSupportDetails details;
    
    // Takes the specified VkPhysicalDevice and VkSurfaceKHR window surface into account when
    // determining the supported capabilities as they are the core components of the swap chain
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mVkSurface, &details.capabilities);

    // Query the supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mVkSurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mVkSurface, &formatCount, details.formats.data());
    }

    // Query the supported presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mVkSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mVkSurface, &presentModeCount, details.presentModes.data());
    }

    // Now we have everything on details, we can return it
    return details;
}

VkSampleCountFlagBits VPhysicalDevice::GetMaxUsableSampleCount() const
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(mVkPhysicalDevice, &physicalDeviceProperties);

    const VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}
