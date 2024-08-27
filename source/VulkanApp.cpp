#include "VulkanApp.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Which validation layer we want to use
const std::vector validationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    // We need to find the address of this function as it's an extension
    const auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    const auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

void VulkanApp::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void VulkanApp::InitWindow()
{
    // Initializes the GLFW library
    glfwInit();

    // We tell them we don't want an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // We don't care about resize for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

bool VulkanApp::IsDeviceSuitable(VkPhysicalDevice_T* device)
{
    // Query for basic device properties like the name, type and supported Vulkan
    //VkPhysicalDeviceProperties deviceProperties;
    //vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Query for support of optional features like texture compression, 64 bit floats and multi viewport rendering
    //VkPhysicalDeviceFeatures deviceFeatures;
    //vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // For now, for example, we return true if we support a geometry shader 
    //return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

    // We check if we have a queue family that works for us
    QueueFamilyIndices indices = FindQueueFamilies(device);

    // Check if we support the extions we need
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    // And then, if we have the correct swapChain support
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

void VulkanApp::PickPhysicalDevice()
{
    // Query for available devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    // If can't find any, there's no need to continue
    // TODO Handle this better
    if (deviceCount == 0)
    {   
        std::cout << "ERROR: Failed to find GPUs with Vulkan support!!" << std::endl;
        return;
    }

    // Get a vector with all physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    // Iterate all devices for one suitable
    for (const auto& physicalDevice : devices)
    {
        if (IsDeviceSuitable(physicalDevice))
        {
            vkPhysicalDevice = physicalDevice;
            return;
        }
    }

    if (vkPhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

QueueFamilyIndices VulkanApp::FindQueueFamilies(VkPhysicalDevice device)
{
    // Logic to find graphics queue family
    QueueFamilyIndices indices;

    // Get the device queue family properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    // And now we fill the vector
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // We need to find one that supports VK_QUEUE_GRAPHICS_BIT
    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        // Check if we have the render capabilities
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        // Now check for a queue for present support
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }
        
        // Early exit
        if (indices.IsComplete()) break;
            
        i++;
    }
    
    return indices;
}

void VulkanApp::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(vkPhysicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies =
    {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    // We will need a queue, for each queue family
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        // Describes the number of queues we want for a single queue family
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        // Priorities to queues to influence the scheduling of command buffer execution
        // using floating point numbers between 0.0 and 1.0
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify is the set of device features that we’ll be using
    VkPhysicalDeviceFeatures deviceFeatures{};
    
    // Now with all this data, we can create the vkDevice
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    // Add pointers to the queue creation info and device features structs
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    // Support for extensions in logical device
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Previous implementations of Vulkan made a distinction between instance and device specific validation layers
    // That means that the enabledLayerCount and ppEnabledLayerNames fields of VkDeviceCreateInfo are ignored by
    // up-to-date implementations. However, we set them anyway
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    // We’re now ready to instantiate the logical device 
    if (vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    // The queues are automatically created along with the logical device
    vkGetDeviceQueue(vkDevice, indices.graphicsFamily.value(), 0, &vkGraphicsQueue);
    vkGetDeviceQueue(vkDevice, indices.presentFamily.value(), 0, &vkPresentQueue);
    
}

// Just checking if a swap chain is available is not sufficient, because it may not actually be compatible with our window surface.
// We need to know
//  * Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
//  * Surface formats (pixel format, color space)
//  * Available presentation modes
SwapChainSupportDetails VulkanApp::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    
    // Takes the specified VkPhysicalDevice and VkSurfaceKHR window surface into account when
    // determining the supported capabilities as they are the core components of the swap chain
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &details.capabilities);

    // Query the supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, details.formats.data());
    }

    // Query the supported presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, details.presentModes.data());
    }

    // Now we have everything on details, we can return it
    return details;
}

bool VulkanApp::CheckDeviceExtensionSupport(VkPhysicalDevice_T* device)
{
    // Get the amount of extensions
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    // And now e get them..it's always the same
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // And we verify we have what we want
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void VulkanApp::CreateSurface()
{
    if (glfwCreateWindowSurface(vkInstance, window, nullptr, &vkSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void VulkanApp::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(vkPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

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

    
    QueueFamilyIndices indices = FindQueueFamilies(vkPhysicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // Next, we need to specify how to handle swap chain images that will be used across multiple queue families
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    // The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the
    // window system. You’ll almost always want to simply ignore the alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // With Vulkan it’s possible that your swap chain becomes invalid or unoptimized while your application is
    // running, for example because the window was resized
    // TOPIC for the future
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // And now we can create the Swap Chain
    if (vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &vkSwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    // We need to retrieve the handles of the VkImages in it
    vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, swapChainImages.data());

    // store the format and extent we’ve chosen for the swap chain images
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanApp::CreateImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];

        // The viewType and format fields specify how the image data should be interpreted.
        // The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        // The components field allows you to swizzle the color channels around.
        // For example, you can map all of the channels to the red channel for a monochrome texture.
        // You can also map constant values of 0 and 1 to a channel. In our case we’ll stick to the default mapping.
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // The subresourceRange field describes what the image’s purpose is and which part of the image should be accessed.
        // Our images will be used as color targets without any mipmapping levels or multiple layers
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void VulkanApp::CreateGraphicsPipeline()
{
    const std::vector<char> vertShaderCode = ReadFile("shaders/vert.spv");
    const std::vector<char> fragShaderCode = ReadFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    // To actually use the shaders we’ll need to assign them to a specific pipeline stage
    // through VkPipelineShaderStageCreateInfo structures as part of the actual pipeline creation process
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    // And here we have the programmable part of the Pipeline
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // And now for the FIXED part

    // VERTEX INPUT:
    // Describes the format of the vertex data that will be passed to the vertex shader in two ways
    //  *Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing)
    //  *Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from and at which offset
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    // Input assembly
    // Describes what kind of geometry will be drawn from the vertices and if primitive restart should be enabled
    // We can have things like VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, etc
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissors
    // A viewport basically describes the region of the framebuffer that the output will be rendered to
    //  Scissor rectangles define in which regions pixels will actually be stored.
    //  Any pixels outside the scissor rectangles will be discarded by the rasterizer

    // We can make then dynamic
    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // And then you only need to specify their count at pipeline creation time:
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform anti-aliasing
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Color Blend
    // Mix the old and new value to produce a final color
    // Combine the old and new value using a bitwise operation
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Pipeline layout for Uniforms
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optiona

    if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // We can now combine everything to create the PIPELINE

    // We start by referencing the array of VkPipelineShaderStageCreateInfo structs
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    // Then we reference all of the structures describing the fixed-function stage.
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    // After that comes the pipeline layout, which is a Vulkan handle rather than a struct pointer.
    pipelineInfo.layout = vkPipelineLayout;

    // And finally we have the reference to the render pass and the index of the sub pass
    pipelineInfo.renderPass = vkRenderPass;
    pipelineInfo.subpass = 0;

    // Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    // FINALLY
    if (vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkGraphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    
    // Cleanup
    vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
}

// Take a buffer with the bytecode as parameter and create a VkShaderModule
VkShaderModule VulkanApp::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VulkanApp::CreateRenderPass()
{
    // We’ll have just a single color buffer attachment represented by one of the images from the swap chain
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // The format of the color attachment should match the format of the swap chain images,
    // and we’re not doing anything with multisampling yet, so we’ll stick to 1 sample
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // Our application won’t do anything with the stencil buffer, so the results of loading and storing are irrelevant.
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // The layout of the pixels in memory can change based on what you’re trying to do with an image.
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpassses
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependencies
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    // We need to wait for the swap chain to finish reading from the image before we can access it
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    // We can finally create the render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanApp::CreateFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    // We’ll then iterate through the image views and create framebuffers from them:
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] =
        {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vkRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void VulkanApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(vkPhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanApp::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // Function that writes the commands we want to execute into a command buffer.

    // We always begin recording a command buffer by calling vkBeginCommandBuffer with a small VkCommandBufferBeginInfo
    // structure as argument that specifies some details about the usage of this specific command buffer.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }


    // Drawing starts by beginning the render pass with vkCmdBeginRenderPass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // We can now bind the graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkGraphicsPipeline);
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // All for this
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
    
}

void VulkanApp::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // So the first frame doesn't need to wait
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (vkCreateSemaphore(vkDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(vkDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(vkDevice, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
}

void VulkanApp::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();

    // Since Vulkan is a platform agnostic API, it can not interface directly with the window system on its own
    // The window surface needs to be created right after the instance creation, because it can actually
    // influence the physical device selection
    CreateSurface();
    
    PickPhysicalDevice();

    // After selecting a physical device to use we need to set up a logical device to interface with i
    CreateLogicalDevice();

    // Now we create the Swap Chain
    CreateSwapChain();
    
    // An image view is quite literally a view into an image. It describes how to access the image and which
    // part of the image to access, for example if it should be treated as a 2D texture depth texture without any mipmapping levels
    CreateImageViews();


    // We need to tell Vulkan about the framebuffer attachments that will be used while rendering.
    // We need to specify how many color and depth buffers there will be, how many samples to use
    // for each of them and how their contents should be handled throughout the rendering operations
    CreateRenderPass();
    
    CreateGraphicsPipeline();

    CreateFramebuffers();

    CreateCommandPool();
    CreateCommandBuffer();

    // Synchronization
    CreateSyncObjects();
}


std::vector<char> VulkanApp::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    // Read all the file, and return a buffer. Simple enough
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void VulkanApp::DrawFrame()
{
    // At a high level, rendering a frame in Vulkan consists of a common set of steps:
    //  1. Wait for the previous frame to finish
    vkWaitForFences(vkDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    // After waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences
    vkResetFences(vkDevice, 1, &inFlightFence);
    
    //  2. Acquire an image from the swakop chain
    uint32_t imageIndex;
    vkAcquireNextImageKHR(vkDevice, vkSwapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    //  3. Record a command buffer which draws the scene onto that image
    vkResetCommandBuffer(commandBuffer, 0);
    RecordCommandBuffer(commandBuffer, imageIndex);
    
    //  4. Submit the recorded command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Specify which semaphores to wait on before execution begins and in which stage(s) of the pipeline to wait
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // Specify which command buffers to actually submit for execution
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Specify which semaphores to signal once the command buffer(s) have finished execution
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Submit the command buffer to the graphics queue 
    if (vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    //  5. Present the swap chain image

    // Specify which semaphores to wait on before presentation can happen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    // Specify the swap chains to present images to and the index of the image for each swap chain
    VkSwapchainKHR swapChains[] = { vkSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    // Submits the request to present an image to the swap chain
    vkQueuePresentKHR(vkPresentQueue, &presentInfo);
}

void VulkanApp::MainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(vkDevice);
}

void VulkanApp::Cleanup() const
{
    // Clean all Sync Objects
    vkDestroySemaphore(vkDevice, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(vkDevice, renderFinishedSemaphore, nullptr);
    vkDestroyFence(vkDevice, inFlightFence, nullptr);
    
    vkDestroyCommandPool(vkDevice, commandPool, nullptr);
    
    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
    }
    
    vkDestroyPipeline(vkDevice, vkGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
    vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
    
    for (const auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(vkDevice, imageView, nullptr);
    }
    
    vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
    vkDestroyDevice(vkDevice, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
    vkDestroyInstance(vkInstance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void VulkanApp::CreateInstance()
{
    // If we want to enableValidationLayers, check if we have them
    if (enableValidationLayers && !CheckValidationLayerSupport())
    {
        std::cout << "ERROR: Validation layers requested, but not available!" << std::endl;
        return;
    }
    
    // We fill some info about our application.
    // Technically optional, but may optimize our app
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Tells the Vulkan driver which global extensions and validation layers we want to use
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Vulkan is a platform agnostic API, which means that you need an extension to interface with the window system.
    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
    // Now we specified everything we need, we can create the Vulkan Instance
    if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
    {
        std::cout << "Error Create Vulkan Instance" << std::endl;
    }
}

void VulkanApp::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // For what kind of severities we want to be called
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    // Which message types you want to be notified
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    // And we finally add the callback  
    createInfo.pfnUserCallback = DebugCallback;
}

void VulkanApp::SetupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

std::vector<const char*> VulkanApp::GetRequiredExtensions()
{
    // We get the required extensions, based if validation layers are enabled or not
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanApp::CheckValidationLayerSupport()
{
    // We get the layerCount
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    // And we get all the available layers
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Now we check if all the validation layers we want are in the available layers
    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

// Choosing the right settings for the Swap Chain
VkSurfaceFormatKHR VulkanApp::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // Each VkSurfaceFormatKHR entry contains a format and a colorSpace membeR
    // VK_FORMAT_B8G8R8A8_SRGB means that we store the B, G, R and alpha channels
    // in that order with an 8 bit unsigned integer for a total of 32 bits per pixel

    // For the color space we’ll use sRGB, which is pretty much the standard color space for viewing and printing purposes
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanApp::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

// The swap extent is the resolution of the swap chain images and it’s almost always
// exactly equal to the resolution of the window that we’re drawing to in pixels
VkExtent2D VulkanApp::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent =
    {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

// VKAPI_ATTR and VKAPI_CALL ensures that we have the right signature
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

