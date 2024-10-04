#pragma once

#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <xstring>
#include <GLFW/glfw3.h>

#include "renderer/VPods.hpp"
#include "renderer/VSwapChain.hpp"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

class VulkanApp
{
public:
    void Run();

private:
    // Model
    std::vector<Renderer::Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;

    // Render Specific
    const int maxFramesInFlight = 2;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    
    GLFWwindow* mGlfwWindow = nullptr;

    VkInstance mVkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    VkPhysicalDevice mVkPhysicalDevice = VK_NULL_HANDLE;
    
    // Set in CreateLogicalDevice
    VkDevice vkDevice = VK_NULL_HANDLE;
    VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
    VkQueue vkPresentQueue = VK_NULL_HANDLE;

    Renderer::VSwapChain::VSwapChain mVSwapChain;
    
    VkSurfaceKHR mVkSurface = VK_NULL_HANDLE;
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
    uint32_t vkMipLevels = 1;
    VkImage vkTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory vkTextureImageMemory = VK_NULL_HANDLE;
    VkImageView vkTextureImageView = VK_NULL_HANDLE;
    VkSampler vkTextureSampler = VK_NULL_HANDLE;

    // Depth Buffer
    VkImage vkDepthImage = VK_NULL_HANDLE;
    VkDeviceMemory vkDepthImageMemory = VK_NULL_HANDLE;
    VkImageView vkDepthImageView = VK_NULL_HANDLE;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    // Multisampling
    VkImage vkColorImage = VK_NULL_HANDLE;
    VkDeviceMemory vkColorImageMemory = VK_NULL_HANDLE;
    VkImageView vkColorImageView = VK_NULL_HANDLE;

    // Helpers
    static std::vector<const char*> GetRequiredExtensions();
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    // Creation Methods
    void InitWindow();
    void InitVulkan();
    void CreateInstance();
    
    void CreateSurface();
    void ReCreateSwapChain();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
    void CreateRenderPass();

    // Drawing
    void CreateFramebuffers();
    void CreateCommandPool();

    // Textures
    void CreateTextureImage();
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    
    void CreateColorResources();
    
    // Buffers
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
    VkCommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    void LoadModel();
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
    
    // Depth Buffer
    static bool HasStencilComponent(VkFormat format);
    VkFormat FindDepthFormat() const;
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    void CreateDepthResources();
    
    void DrawFrame();
    void MainLoop();
    
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
