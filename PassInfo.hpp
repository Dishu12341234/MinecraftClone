#ifndef PASSINFO_HPP
#define PASSINFO_HPP

#define GLFW_INCLUDE_VULKAN
#include <string>
#include <GLFW/glfw3.h>

struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool commandPool;
};

struct u_GraphicsPipelineCreateInfo
{
    VkDevice device;
    int height, width;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;
    VkDescriptorSetLayout* descriptorSetLayout;
    VkSampleCountFlagBits msaaSamples;
};

struct u_TexturePassInfo
{
    int height, width;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;
    VkCommandPool commandPool;
    std::string texturePath;
};


#endif