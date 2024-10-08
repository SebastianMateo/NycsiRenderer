#include "VImage.h"

#include <cstdint>
#include <stdexcept>

#include "VPhysicalDevice.hpp"
#include "vulkan/vulkan_core.h"

namespace Renderer::VImage
{
    VImageHandler CreateImage(
        const VkPhysicalDevice vkPhsyicalDevice,
        const VkDevice vkDevice,
        const uint32_t width,
        const uint32_t height,
        const VImageProperties& imageProperties)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = imageProperties.mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = imageProperties.format;
        imageInfo.tiling = imageProperties.tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = imageProperties.usage;
        imageInfo.samples = imageProperties.numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VImageHandler imageHandler;
        if (vkCreateImage(vkDevice, &imageInfo, nullptr, &imageHandler.image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vkDevice, imageHandler.image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VPhysicalDevice::FindMemoryType(vkPhsyicalDevice, memRequirements.memoryTypeBits, imageProperties.properties);

        if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &imageHandler.imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(vkDevice, imageHandler.image, imageHandler.imageMemory, 0);
        return imageHandler;
    }
}
