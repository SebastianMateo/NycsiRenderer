#include "VSwapChain.hpp"

#include <algorithm>
#include <iostream>

#include "VPhysicalDevice.hpp"
#include "VPods.hpp"
#include "GLFW/glfw3.h"

namespace Renderer::VSwapChain
{
    // Choosing the right settings for the Swap Chain
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    // The swap extent is the resolution of the swap chain images
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    // Represents the actual conditions for showing images to the screen
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VSwapChain CreateSwapChain(
        const VkPhysicalDevice vkPhysicalDevice,
        const VkSurfaceKHR vkSurface,
        const VkDevice vkDevice,
        GLFWwindow* glfwWindow)
    {
        const SwapChainSupportDetails swapChainSupport = VPhysicalDevice::QuerySwapChainSupport(vkPhysicalDevice, vkSurface);

        const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        const VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        const VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, glfwWindow);

        // Aside from these properties we also have to decide how many images we would like to have in the swap chain.
        // The implementation specifies the minimum number that it requires to function:
        
        // However, simply sticking to this minimum means that we may sometimes have to wait on the driver to
        // complete internal operations before we can acquire another image to render to. Therefore it is
        // recommended to request at least one more image than the minimum:
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        // We should also make sure to not exceed the maximum number of images while doing this,
        // where 0 is a special value that means that there is no maximum
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // As per usual, we fill a struct
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = vkSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        const auto [graphicsFamily, presentFamily] = VPhysicalDevice::FindQueueFamilies(vkPhysicalDevice, vkSurface);
        const uint32_t queueFamilyIndices[] = {graphicsFamily.value(), presentFamily.value()};

        // Next, we need to specify how to handle swap chain images that will be used across multiple queue families
        if (graphicsFamily != presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        // The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the
        // window system. You'll almost always want to simply ignore the alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        
        // And now we can create the Swap Chain
        VSwapChain vSwapChain;
        if (vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &vSwapChain.vkSwapChain) != VK_SUCCESS)
        {
            std::cout << "Failed to create swap chain!" << '\n';
        }

        // We need to retrieve the handles of the VkImages in it
        vkGetSwapchainImagesKHR(vkDevice, vSwapChain.vkSwapChain, &imageCount, nullptr);
        vSwapChain.swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vkDevice, vSwapChain.vkSwapChain, &imageCount, vSwapChain.swapChainImages.data());

        // Store the format and extent we've chosen for the swap chain images
        vSwapChain.swapChainImageFormat = surfaceFormat.format;
        vSwapChain.swapChainExtent = extent;

        // And we create the image views
        vSwapChain.swapChainImageViews.resize(vSwapChain.swapChainImages.size());

        for (size_t i = 0; i < vSwapChain.swapChainImages.size(); i++)
        {
            vSwapChain.swapChainImageViews[i] = CreateImageView(vkDevice, vSwapChain.swapChainImages[i], vSwapChain.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
        
        return vSwapChain;
    }

    void CleanupSwapChain(
        const VkDevice vkDevice,
        const VkImageView vkColorImageView,
        const VkImage vkColorImage,
        const VkDeviceMemory vkColorImageMemory,
        const VSwapChain& vSwapChain,
        const std::vector<VkFramebuffer>& swapChainFramebuffers)
    {
        vkDestroyImageView(vkDevice, vkColorImageView, nullptr);
        vkDestroyImage(vkDevice, vkColorImage, nullptr);
        vkFreeMemory(vkDevice, vkColorImageMemory, nullptr);
    
        for (const VkFramebuffer framebuffer : swapChainFramebuffers)
        {
            vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
        }

        for (const VkImageView imageView : vSwapChain.swapChainImageViews)
        {
            vkDestroyImageView(vkDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(vkDevice, vSwapChain.vkSwapChain, nullptr);
    }

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        // Each VkSurfaceFormatKHR entry contains a format and a colorSpace member
        // VK_FORMAT_B8G8R8A8_SRGB means that we store the B, G, R and alpha channels
        // in that order with an 8 bit unsigned integer for a total of 32 bits per pixel

        // For the color space we'll use sRGB, which is pretty much the standard color space for viewing and printing purposes
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            //  This mode can be used to render frames as fast as possible while still avoiding tearing,
            //  resulting in fewer latency issues than standard vertical sync. This is commonly known as "triple buffering"
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        // Only the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
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
