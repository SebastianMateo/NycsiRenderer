#pragma once

#define GLFW_INCLUDE_VULKAN
#include <optional>
#include <vector>
#include <xstring>
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

// It’s actually possible that the queue families supporting drawing commands
// and the ones supporting presentation do not overlap.
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool IsComplete() const
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

class VulkanApp
{
public:
    void Run();

private:
    GLFWwindow* window = nullptr;
    VkInstance vkInstance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    QueueFamilyIndices graphicsFamily;
    VkDevice vkDevice;
    VkSwapchainKHR vkSwapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

    VkQueue vkGraphicsQueue;
    VkQueue vkPresentQueue;
    VkSurfaceKHR vkSurface;
    VkRenderPass vkRenderPass;
    VkPipelineLayout vkPipelineLayout;
    VkPipeline vkGraphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // Synchronization
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    
    void InitWindow();

    // Physical Device Creation
    void PickPhysicalDevice();
    bool IsDeviceSuitable(VkPhysicalDevice_T* device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    // Logical Device Creation
    void CreateLogicalDevice();
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice_T* device);
    
    void CreateSurface();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void CreateSyncObjects();
    void InitVulkan();
    static std::vector<char> ReadFile(const std::string& filename);

    void DrawFrame();
    void MainLoop();
    
    void Cleanup() const;
    void CreateInstance();
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void SetupDebugMessenger();

    std::vector<const char*> GetRequiredExtensions();
    bool CheckValidationLayerSupport();
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    static VkBool32 DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
        );
};