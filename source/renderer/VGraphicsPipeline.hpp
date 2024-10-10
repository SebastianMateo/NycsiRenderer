#pragma once
#include <vector>

#include "vulkan/vulkan_core.h"

namespace Renderer::GraphicsPipeline
{
    struct VGraphicsPipeline
    {
        VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
        VkPipeline vkGraphicsPipeline = VK_NULL_HANDLE;    
    };
    
    VGraphicsPipeline CreateGraphicsPipeline(
        VkDevice vkDevice,
        VkDescriptorSetLayout vkDescriptorSetLayout,
        VkRenderPass vkRenderPass,
        VkSampleCountFlagBits msaaSamples
    );
    
    VkShaderModule CreateShaderModule(VkDevice vkDevice, const std::vector<char>& code);
};
