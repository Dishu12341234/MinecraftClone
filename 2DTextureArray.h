#pragma once
#include "Textures.hpp"
#include <vector>

class TextureArray2D
// A class to create a new texture array 2D with ease
// To upload an image call the function with the appropriate file path
{
private:
  VulkanContext &vkContext;

  VkImage blkTextureArrayImage;
  VkDeviceMemory blkTextureArrayMemory;
  VkImageView blkTextureArrayImageView;

  VkBuffer stagingBlkBuffer;
  VkDeviceMemory stagingBlkBufferMemory;

  VkSampler blkArraySampler = VK_NULL_HANDLE;

  std::vector<unsigned char *> blockTextures;

  VkQueue graphicsQueue;
  VkExtent2D swapChainExtent;
  VkRenderPass renderPass;
  VkCommandPool commandPool;

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);

  VkCommandBuffer beginSingleTimeCommands();

  void endSingleTimeCommands(VkCommandBuffer commandBuffer);

  friend class HelloTriangleApplication;

  int layerCount = 512;
  uint32_t textureWidth = 16;
  uint32_t textureHeight = 16;

  std::vector<std::string> filePaths;

public:
  TextureArray2D(VulkanContext &vkContext, int layerCount = 32, int w = 16,
                 int h = 16);

  void passInfo(u_TexturePassInfo &passInfo);

  void createBlkTextureArrayImage();
  void allocateBlkTextureArrayMemory();
  void createBlkTextureArrayImageView();
  void transitionBlkTextureArrayImageLayout();
  void createBlkTextureStagingBuffer();
  void createBlkArraySampler();

  void setTexturePaths(uint32_t idx, std::string path);
  void allocateStorageForTextures();

  void uploadBlkTexturesToArray();
  void transitionBlkTextureArrayToShaderRead();

  void cleanUp();

  ~TextureArray2D();
};
