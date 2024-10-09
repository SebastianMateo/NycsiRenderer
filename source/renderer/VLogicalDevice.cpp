#include "VLogicalDevice.hpp"

#include <iostream>
#include <set>

#include "VPhysicalDevice.hpp"
#include "VPods.hpp"
#include "VSwapChain.hpp"

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

    VkRenderPass CreateRenderPass(
        VkPhysicalDevice vkPhysicalDevice,
        VkDevice vkDevice,
        VSwapChain::VSwapChain& swapChain,
        VkSampleCountFlagBits msaaSamples)
    {
        // Now we are using multisampling
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChain.swapChainImageFormat;
        colorAttachment.samples = msaaSamples;

        // The format of the color attachment should match the format of the swap chain images,
        // and we’re not doing anything with multisampling yet, so we’ll stick to 1 sample
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // Our application won’t do anything with the stencil buffer, so the results of loading and storing are irrelevant.
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // The layout of the pixels in memory can change based on what you’re trying to do with an image.
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // because multisampled images cannot be presented directly

        // Depth
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VPhysicalDevice::FindDepthFormat(vkPhysicalDevice);
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = swapChain.swapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        // Subpassses
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;
        
        // Subpass dependencies
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        
        // We can finally create the render pass
        std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkRenderPass vkRenderPass;
        if (vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS)
        {
            std::cout << "Failed to create render pass!" << '\n';
        }

        return vkRenderPass;
    }
}
