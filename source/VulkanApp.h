#pragma once

#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <xstring>
#include <GLFW/glfw3.h>

#include "renderer/VGraphicsPipeline.hpp"
#include "renderer/VImage.hpp"
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
    VkDevice mVkDevice = VK_NULL_HANDLE;
    VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
    VkQueue vkPresentQueue = VK_NULL_HANDLE;

    Renderer::VSwapChain::VSwapChain mVSwapChain;
    Renderer::GraphicsPipeline::VGraphicsPipeline mVkGraphicsPipeline;
    
    VkSurfaceKHR mVkSurface = VK_NULL_HANDLE;
    VkRenderPass mVkRenderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout mVkDescriptorSetLayout = VK_NULL_HANDLE; 
    
    std::vector<VkFramebuffer> mSwapChainFramebuffers;
    
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
    Renderer::VImage::VImageHandler mTextureImageHandler;
    VkImageView vkTextureImageView = VK_NULL_HANDLE;
    VkSampler vkTextureSampler = VK_NULL_HANDLE;

    // Depth Buffer
    Renderer::VImage::VImageHandler mDepthImageHandler;
    VkImageView vkDepthImageView = VK_NULL_HANDLE; // This probably should go with the image? maybe
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    // Multisampling
    Renderer::VImage::VImageHandler mColorImageHandler;
    VkImageView vkColorImageView = VK_NULL_HANDLE;
    
    // Helpers
    static std::vector<const char*> GetRequiredExtensions();

    // Creation Methods
    void InitWindow();
    void InitVulkan();
    void CreateInstance();
    
    void ReCreateSwapChain();
    
    // Drawing
    void CreateCommandPool();

    // Textures
    void CreateTextureImage();
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;
    void CreateTextureSampler();
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
    void UpdateUniformBuffer(uint32_t currentImage) const;
    
    // Depth Buffer
    static bool HasStencilComponent(VkFormat format);
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
