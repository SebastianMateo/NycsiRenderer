#include "VLogicalDevice.hpp"

#include <iostream>
#include <set>

#include "VPhysicalDevice.hpp"
#include "VPods.hpp"

namespace Renderer::VLogicalDevice
{
    VkDevice CreateLogicalDevice(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
    {
        VkDevice vkDevice = VK_NULL_HANDLE;
        
        // We get our queue families we are going to use
        auto [graphicsFamily, presentFamily] = VPhysicalDevice::FindQueueFamilies(vkPhysicalDevice, vkSurface);
        std::set uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };  // NOLINT(bugprone-unchecked-optional-access)

        // And create a Queue for each family
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            // Describes the number of queues we want for a single queue family
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority; // Between 0.0 and 1.0
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Specify device features we will be using
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        
        // Now with all this data, we can create the vkDevice
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        // Add pointers to the queue creation info and device features structs
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        // Support for extensions in logical device
        createInfo.enabledExtensionCount = static_cast<uint32_t>(VPhysicalDevice::DEVICE_EXTENSIONS.size());
        createInfo.ppEnabledExtensionNames = VPhysicalDevice::DEVICE_EXTENSIONS.data();

        // Previous implementations of Vulkan made a distinction between instance and device specific validation layers
        // That means that the enabledLayerCount and ppEnabledLayerNames fields of VkDeviceCreateInfo are ignored by
        // up-to-date implementations. However, we set them anyway
        #ifdef _DEBUG
            createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        #elif
            createInfo.enabledLayerCount = 0;
        #endif
        
        // We’re now ready to instantiate the logical device 
        if (vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS)
        {
            std::cout << "failed to create logical device!" << '\n';
        }
        
        return vkDevice;
    }
}
