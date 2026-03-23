#include "HelloTriangleApplication.hpp"
#include "Event.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <map>
#include <set>

VkSampleCountFlagBits HelloTriangleApplication::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    // if (counts & VK_SAMPLE_COUNT_64_BIT)
    // {
    //     return VK_SAMPLE_COUNT_64_BIT;
    // }
    // if (counts & VK_SAMPLE_COUNT_32_BIT)
    // {
    //     return VK_SAMPLE_COUNT_32_BIT;
    // }
    // if (counts & VK_SAMPLE_COUNT_16_BIT)
    // {
    //     return VK_SAMPLE_COUNT_16_BIT;
    // }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

void HelloTriangleApplication::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    _window = glfwCreateWindow(WIDTH, HEIGHT, PROCESS_NAME.c_str(), nullptr, nullptr);

    this->event = std::make_unique<Event>(*_window);
    glfwSetWindowUserPointer(_window, event.get());

    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
    createSurface();

    
    pickPhysicalDevice();
    isDeviceSuitable(physicalDevice);
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();

    createDescriptorSetLayout();

    u_GraphicsPipelineCreateInfo createInfo{};
    createInfo.device = device;
    createInfo.height = HEIGHT;
    createInfo.width = WIDTH;
    createInfo.msaaSamples = msaaSamples;
    createInfo.swapChainExtent = swapChainExtent;
    createInfo.renderPass = renderPass;
    createInfo.descriptorSetLayout = &descriptorSetLayout;

    graphicsPipeline.u_PassGraphicsPipelineCreateInfo(createInfo);
    graphicsPipeline.createGraphicsPipeline();

    rayGraphicsPipeline.u_PassGraphicsPipelineCreateInfo(createInfo);
    rayGraphicsPipeline.createGraphicsPipeline();

    uiRenderPipeline.u_PassGraphicsPipelineCreateInfo(createInfo);
    uiRenderPipeline.createGraphicsPipeline();
    u_TexturePassInfo texturePassInfo{};
    texturePassInfo.device = device;
    texturePassInfo.physicalDevice = physicalDevice;
    texturePassInfo.swapChainExtent = swapChainExtent;

    for (size_t i = 0; i < NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES; i++)
    {
        uiTexturePaths[i] = "textures/corrupt.png";
    }
    

    for (size_t i = 0; i < NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES; i++)
    {
        uiTextures.emplace_back();
    }

    texture.passTextureCreateInfo(texturePassInfo);
    for (size_t i = 0; i < NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES; i++)
    {
        uiTextures.at(i).passTextureCreateInfo(texturePassInfo);
    }

    createColorResources();
    createDepthResources();
    createFramebuffers();
    createCommandPool();

    texturePassInfo.graphicsQueue = graphicsQueue;
    texturePassInfo.renderPass = renderPass;
    texturePassInfo.commandPool = commandPool;
    texturePassInfo.height = HEIGHT;
    texturePassInfo.width = WIDTH;
    texturePassInfo.texturePath = TEXTURE_PATH;
    texture.passTextureCreateInfo(texturePassInfo);


    uiTexturePaths[0] = "textures/inventory.png";
    uiTexturePaths[1] = "textures/crosshair.png";
    for (size_t i = 0; i < NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES; i++)
    {
        texturePassInfo.texturePath = uiTexturePaths[i];
        uiTextures.at(i).passTextureCreateInfo(texturePassInfo);
    }

    loadModel();
    // createVertexBuffer();
    // createIndexBuffer();
    initGameObjects();
    createUniformBuffers();
    createDescriptorPool();

    std::cout << "Creating Texture..." << std::endl;
    texture.createTextureImage();
    texture.createTextureView();
    texture.createTextureSampler();
    for (size_t i = 0; i < NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES; i++)
    {
        uiTextures.at(i).createTextureImage();
        uiTextures.at(i).createTextureView();
        uiTextures.at(i).createTextureSampler();
    }

    createDescriptorSets();
    createCommandBuffers();
    createSyncObject();
}

void HelloTriangleApplication::createInstance()
{
    std::cout << "Creating Vulkan Instance..." << std::endl;
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested, but not available");
    }

    if (enableValidationLayers)
    {
        std::cout << "Validation layers enabled" << std::endl;
    }

    // --- Application info ---
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1; // Use 1.1 for MoltenVK

    // --- Instance create info ---
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // --- Validation layers ---
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    // --- Required extensions ---
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // --- Create instance ---
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }

    // --- Print available extensions ---
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "available extensions:\n";
    for (const auto &extension : extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

// finding if VL_LAYERS_KHRONOS_validation is there or not
bool HelloTriangleApplication::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (auto &layerPorperties : availableLayers)
        {
            if (strcmp(layerName, layerPorperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

void HelloTriangleApplication::createSurface()
{
    std::cout << "Creating window surface..." << std::endl;
    if (glfwCreateWindowSurface(instance, _window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Window surface");
    }
}

void HelloTriangleApplication::pickPhysicalDevice()
{
    std::cout << "Picking Physical Device..." << std::endl;
    physicalDevice = VK_NULL_HANDLE; // this will be implicitly cleanup when VkInstance is destroid(no additoinal cleanup needed)

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs that support Vulkan");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        int score = rateDeviceSuitablity(device);
        candidates.insert(std::make_pair(score, device));
        std::cout << "Score: " << score << std::endl;
    }

    if (candidates.rbegin()->first > 0)
    {
        physicalDevice = candidates.rbegin()->second;
        msaaSamples = getMaxUsableSampleCount();
    }
    else
    {
        throw std::runtime_error("Failed to find a suitable GPU");
    }
}

// right now we only allow to use dedicated cards the support geometry shaders
// bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
// {

//     // this controls which type of GPU to allow
//     return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
// }

int HelloTriangleApplication::rateDeviceSuitablity(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceProperties(device, &props);
    vkGetPhysicalDeviceFeatures(device, &feats);

    int score = 0;

    std::cout << "Device: " << props.deviceName << std::endl;

    // Prefer discrete or integrated GPU
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 10000;
    else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        score += 5000;

    // Bigger textures = better
    score += props.limits.maxImageDimension2D;

    // Useful features
    if (feats.samplerAnisotropy)
        score += 1000;
    if (feats.shaderInt64)
        score += 500;
    if (feats.wideLines)
        score += 300;

#ifdef __APPLE__
    // Metal backend doesn’t support geometry/tessellation shaders
    score += 1500;
#else
    if (!feats.geometryShader)
        return 0; // skip if no geometry shader
#endif

    return score;
}

// Queue -> a line in which a set of commands needs to be executed
// there maybe multiple types of queues(Graphics, Computem, ...)
// Queue Family -> Conatains multiple queues for the GPU(Note: A queue family must be homogenous)

// finding queue families
HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // grts all the queue families supported by the GPU

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }
    // NOTE: some GPU supports different graphics and presentation queues

    return indices;
}

// Logical device -> A brige between the program and the GPU
void HelloTriangleApplication::createLogicalDevice()
{
    std::cout << "Creating Logical Device..." << std::endl;
    // Queue Family Stuff
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice); // get the required queue families
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // right now only creating a single queue(can create muliple)
    float queuePriority = 1.f;
    for (uint32_t queueFamily : uniqueQueueFamilies) // prepare each queue family
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo); // push the queue info in the vector
    }

    // ---- Specifing which feature we wanna use ----
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

    // If you're on Vulkan 1.2, use VkPhysicalDeviceVulkan12Features instead:
    VkPhysicalDeviceVulkan12Features vk12Features{};
    vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk12Features.descriptorIndexing                        = VK_TRUE;
    vk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vk12Features.pNext = nullptr;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.pNext = &vk12Features;

#ifdef __APPLE__
    deviceExtensions.push_back("VK_KHR_portability_subset");
#else
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#endif

    deviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Device specific validation layers
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Faild to create logical device");
    }

    // ---- retrive the queues and store them into the class members ----
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

// this is never called because I have rateDeviceSutablity
// this is just redundent
bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    VkPhysicalDeviceDescriptorIndexingFeatures indexingSupport{};
    indexingSupport.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &indexingSupport;

    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

    if (!indexingSupport.shaderSampledImageArrayNonUniformIndexing)
    {
        throw std::runtime_error("Device does not support non-uniform texture indexing!");
    }

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    // ---- counting the number of extensions ----
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtansions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtansions.data()); // all the required extensions are now retrived

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtansions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    // if all the required extensions are supported then the 'requiredExtension' vector is empty
    // which returns true
    return requiredExtensions.empty();
}

// Swapchain -> A queue of images presented to the screen in a sequence
// Draw to an offscreen -> present it to the current window -> repeat

// This function asks the GPU about the capablitie of the SwapChain that this GPU has
HelloTriangleApplication::SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    // ---- Get the capabilities ----
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // ---- get the format ----
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // ---- get the presentation mode(vsync, mailbox ...) ----
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

// A swap suface define how pixels are stored(RGB vs BRGA or linear vs sRGB)
VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats.at(0);
    }
}

// this specifies how to presnet the images to the screen (vsync or mailbox or normal)
VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
}

// Extent defines the resolution of the image
// MacOS has a fixed extent while other windowing systems have a varible extent which is clamped to the window bounds
// also High-DPI displays like Apple Liquid Retina displays have to be configure in a sepreate way
VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
#ifdef __APPLE__
    // MoltenVK on macOS (Retina-safe)
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)};

    return actualExtent; // Always use framebuffer size on Retina
#else
    // Metal IE: the size is predifined and is not UINT 32 max. therefore, use the size provided by the system
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    // The surface does not have a fixed size and you must create it by yourself
    else
    {
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        VkExtent2D actualExtent =
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

#endif
}

// here we are using a staging buffer which is accisible by the CPU and then afterwards copying the contents of the staging buffer to the vertex buffer
void HelloTriangleApplication::createVertexBuffer()
{
    std::cout << "Creating Vertex Buffer..." << std::endl;

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // TX
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, stagingBuffer, stagingBufferMemory);

    // filling in the vertex buffer
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // creating the actul VBO and setting it up so the it is the destionation of the transfer source
    // TX
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    // but we still have to copy the memory contents manully
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createIndexBuffer()
{
    std::cout << "Creating Index Buffer..." << std::endl;

    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createUniformBuffers()
{
    std::cout << "Creating Uniform Buffers..." << std::endl;

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT); // like 'void *data;'

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void HelloTriangleApplication::createDescriptorPool()
{
    std::cout << "Creating Descriptor Pool..." << std::endl;

    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES * static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // uiTexturesBinding.descriptorCount = 16;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void HelloTriangleApplication::createDescriptorSets()
{
    std::cout << "Creating Descriptor Sets..." << std::endl;

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    updateDescriptorSets(texture);
}

void HelloTriangleApplication::updateDescriptorSets(u_Texture &texture)
{

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.textureImageView;
        imageInfo.sampler = texture.textureSampler;

        std::vector<VkDescriptorImageInfo> imageInfos(NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES);
        for (uint32_t i = 0; i < NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES; i++)
        {
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[i].imageView = uiTextures[i].textureImageView; // your VkImageView array
            imageInfos[i].sampler = uiTextures[i].textureSampler;
        }

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES;
        descriptorWrites[2].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void HelloTriangleApplication::createDescriptorSetLayout()
{
    std::cout << "Creating Descriptor Set Layout..." << std::endl;

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding uiTexturesBinding{};
    uiTexturesBinding.binding = 2;
    uiTexturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uiTexturesBinding.descriptorCount = NUM_DESCRIPTOR_COUNT_FOR_UI_TEXTURES;
    uiTexturesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uiTexturesBinding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = 3;
    flagsInfo.pBindingFlags = &bindingFlags;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding, samplerLayoutBinding, uiTexturesBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.pNext = nullptr;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

// Memory trasnfer opertation are done using command buffer just like drawing
void HelloTriangleApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

// A command buffer is a queue of commands to send to the GPU
void HelloTriangleApplication::createCommandBuffers()
{
    std::cout << "Creating Command Buffers..." << std::endl;

    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers");
    }
}
void HelloTriangleApplication::createSyncObject()
{
    // 1️⃣ Per-frame semaphores for acquiring images
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    // 2️⃣ Per-swapchain-image semaphores for rendering completion
    renderFinishedSemaphores.resize(swapChainImages.size());

    // 3️⃣ Fences to limit frames in flight
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled

    // Create per-frame semaphores and fences
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphore or fence!");
        }
    }

    // Create per-swapchain-image render-finished semaphores
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create renderFinished semaphore!");
        }
    }
}
