#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "PassInfo.hpp"
#include "include/stb-master/stb_image.h"

class u_Texture
{
private:
    int height, width;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;
    VkCommandPool commandPool;

public:
    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    std::string texturePath;
    u_Texture();
    ~u_Texture();
    void passTextureCreateInfo(const u_TexturePassInfo &pCreateInfo);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    static void createBuffer(VulkanContext vkContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits msaaSamples, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    void createTextureImage();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    void createTextureView();
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    void createTextureSampler();

    void destroy();
};

#endif