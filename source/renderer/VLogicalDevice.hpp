#pragma once
#include "vulkan/vulkan_core.h"

namespace Renderer::VLogicalDevice
{
    VkDevice CreateLogicalDevice(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);   
}
