#pragma once

#include <optional>
#include <vector>
#include "vulkan/vulkan_core.h"

namespace Renderer
{
    #ifdef _DEBUG
        // Which validation layer we want to use
        const std::vector VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    #endif

    // Possible that graphic and supporting family do not overlap
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool IsComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };    
}
