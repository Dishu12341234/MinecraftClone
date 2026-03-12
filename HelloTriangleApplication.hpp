#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <chrono>
#include "Textures.hpp"
#include "GraphicsPipeline.h"
#include "GameMeshObject.h"
#include "MeshUploader.h"
#include "GameObjectPool.h"

class HelloTriangleApplication
{
private:
    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    VkInstance instance;
    MeshUploader meshUploader;

    GameObjectPool gameObjectPool;
    GameMeshObject gmo1;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VkSurfaceKHR surface;
    VkQueue presentQueue;

    VkPhysicalDevice physicalDevice;
    VkDevice device;       // logical device
    VkQueue graphicsQueue; // implicitly cleaned up

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages; // implicitly cleaned up
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews; // explicit cleanup
    VkCommandPool commandPool;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    GameMeshObject testGameObject;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Vertex Buffer(VBO)
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    //MSAA
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    void initWindow();

    void initVulkan();
    void createInstance();

    bool checkValidationLayerSupport();

    void createSurface();

    void pickPhysicalDevice();
    int rateDeviceSuitablity(VkPhysicalDevice device);

    void createLogicalDevice();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    // Surface format
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capablities);

    VkRenderPass renderPass;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::vector<VkCommandBuffer> commandBuffers;

    void createSwapChain();
    void createImageViews();
    void createRenderPass();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkDescriptorSetLayout descriptorSetLayout;
    void createDescriptorSetLayout();

    u_GraphicsPipeline graphicsPipeline;

    void initGameObjects();

    void createFramebuffers();
    void createCommandPool();

    void createColorResources();
    void createDepthResources();

    u_Texture texture;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void loadModel();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    void createDescriptorSets();

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBufferm, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createCommandBuffers();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createSyncObject();

    void mainLoop();

    void updateUniformBuffer(uint32_t currentImage);

    void drawFrame();

    void cleanup();

    const int WIDTH = 1080;
    const int HEIGHT = 720;

    std::string PROCESS_NAME = "Vulkos";

    const std::string MODEL_PATH = "models/Cube.obj";
    const std::string TEXTURE_PATH = "textures/atlas.png";
    

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifndef NDEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

    GLFWwindow *_window;

public:
    HelloTriangleApplication();
    HelloTriangleApplication(std::string processName);
    void run();
    ~HelloTriangleApplication();
};

// Future todos:
// Allocate a single buffer and re use it later using offstes(Vertex buffer 2.4)
// And also same for memory allocation(Vertex buffer 2.2)