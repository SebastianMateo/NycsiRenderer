#pragma once

#define GLFW_INCLUDE_VULKAN
#include <array>
#include <optional>
#include <vector>
#include <xstring>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

// It’s actually possible that the queue families supporting drawing commands
// and the ones supporting presentation do not overlap.
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

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        return bindingDescription;
    }

    // Describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description
    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        // Position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        // Color
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        // Texture
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
};

class VulkanApp
{
public:
    void Run();

private:
    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    
    GLFWwindow* window = nullptr;

    VkInstance vkInstance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;

    // Set in SelectPhysicalDevice
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;

    // Set in CreateLogicalDevice
    VkDevice vkDevice = VK_NULL_HANDLE;
    VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
    VkQueue vkPresentQueue = VK_NULL_HANDLE;
    
    VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent = {};
    std::vector<VkImageView> swapChainImageViews;
    
    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE; 
    VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
    VkPipeline vkGraphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    VkCommandPool vkCommandPool = VK_NULL_HANDLE;

    // We need to have multiple to handle multiple frames in flight
    std::vector<VkCommandBuffer> vkCommandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    // Buffers
    VkBuffer vkVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkVertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer vkIndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkIndexBufferMemory = VK_NULL_HANDLE;

    // Uniform buffers. We need as many as frames in flight
    std::vector<VkBuffer> vkUniformBuffers;
    std::vector<VkDeviceMemory> vkUniformBuffersMemory;
    std::vector<void*> vkUniformBuffersMapped;
    VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> vkDescriptorSets;

    // Texture
    VkImage vkTextureImage;
    VkDeviceMemory vkTextureImageMemory;
    VkImageView vkTextureImageView;
    VkSampler vkTextureSampler;

    // Helpers
    static std::vector<const char*> GetRequiredExtensions();
    bool IsDeviceSuitable(VkPhysicalDevice_T* device) const;
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice_T* device);

    // Checking if a swap chain is available is not sufficient, because it may not actually be compatible with our window surface.
    // We need to know
    //  * Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
    //  * Surface formats (pixel format, color space)
    //  * Available presentation modes
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
    // Choosing the right settings for the Swap Chain
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    // Represents the actual conditions for showing images to the screen
    static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    // The swap extent is the resolution of the swap chain images
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    // Creation Methods
    void InitWindow();
    void InitVulkan();
    void CreateInstance();
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface();
    void CreateSwapChain();
    void ReCreateSwapChain();
    VkImageView CreateImageView(VkImage image, VkFormat format) const;
    void CreateImageViews();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    [[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
    void CreateRenderPass();

    // Drawing
    void CreateFramebuffers();
    void CreateCommandPool();

    // Textures
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    
    // Buffers
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
    VkCommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;
    
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void CreateSyncObjects();
    static std::vector<char> ReadFile(const std::string& filename);
    void UpdateUniformBuffer(uint32_t currentImage) const;
    
    void DrawFrame();
    void MainLoop();

    void CleanupSwapChain() const;
    void Cleanup() const;
    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void SetupDebugMessenger();

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    
    static bool CheckValidationLayerSupport();
    static VkBool32 DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
        );
};
