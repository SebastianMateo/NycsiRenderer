#include "VImage.hpp"

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

    VkImageView CreateImageView(
        const VkDevice vkDevice,
        const VkImage image,
        const VkFormat format,
        const VkImageAspectFlags aspectFlags,
        const uint32_t mipLevels)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
    
        // The viewType and format fields specify how the image data should be interpreted.
        // The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
    
        // The subresourceRange field describes what the image's purpose is and which part of the image should be accessed.
        viewInfo.subresourceRange.aspectMask = aspectFlags; //VK_IMAGE_ASPECT_DEPTH_BIT or VK_IMAGE_ASPECT_COLOR_BIT for example
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image view!");
        }

        return imageView;
    }
}
