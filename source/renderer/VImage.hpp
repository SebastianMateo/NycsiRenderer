#pragma once
#include "vulkan/vulkan_core.h"

namespace Renderer::VImage
{
    // Struct that handles a buffer for an Image and the Device Memory
    struct VImageHandler
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE; 
    };

    // Common Image Properties
    struct VImageProperties
    {
        uint32_t mipLevels = 1;
        VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        VkFormat format = VK_FORMAT_MAX_ENUM;
        VkImageTiling tiling = VK_IMAGE_TILING_MAX_ENUM;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
    };
    
    VImageHandler CreateImage(
        VkPhysicalDevice vkPhsyicalDevice,
        VkDevice vkDevice,
        uint32_t width,
        uint32_t height,
        const VImageProperties& imageProperties
    );

    VkImageView CreateImageView(
        const VkDevice vkDevice,
        const VkImage image,
        const VkFormat format,
        const VkImageAspectFlags aspectFlags,
        const uint32_t mipLevels
    );
};


