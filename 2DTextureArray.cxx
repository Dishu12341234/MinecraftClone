#include "2DTextureArray.h"
#include <cstring>
#include <stdexcept>

void TextureArray2D::passInfo(u_TexturePassInfo &passInfo) {
  this->commandPool = passInfo.commandPool;
  this->graphicsQueue = passInfo.graphicsQueue;
  this->renderPass = passInfo.renderPass;
  this->swapChainExtent = passInfo.swapChainExtent;

  blockTextures.resize(layerCount);
  filePaths.resize(layerCount);

  std::fill(filePaths.begin(), filePaths.end(),
            "/home/divyansh/MinecraftClone/textures/corrupt_blk.png");
}

VkCommandBuffer TextureArray2D::beginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(vkContext.device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void TextureArray2D::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);

  vkFreeCommandBuffers(vkContext.device, commandPool, 1, &commandBuffer);
}

uint32_t TextureArray2D::findMemoryType(uint32_t typeFilter,
                                        VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(vkContext.physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

TextureArray2D::TextureArray2D(VulkanContext &vkContext, int layerCount, int w,
                               int h)
    : vkContext(vkContext), layerCount(layerCount), textureWidth(w),
      textureHeight(h) {}

void TextureArray2D::createBlkTextureArrayImage() {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent = {textureWidth, textureHeight, 1};

  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = this->layerCount;

  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage =
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkCreateImage(vkContext.device, &imageInfo, nullptr, &blkTextureArrayImage);
}

void TextureArray2D::allocateBlkTextureArrayMemory() {
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(vkContext.device, blkTextureArrayImage,
                               &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vkAllocateMemory(vkContext.device, &allocInfo, nullptr,
                   &blkTextureArrayMemory);
  vkBindImageMemory(vkContext.device, blkTextureArrayImage,
                    blkTextureArrayMemory, 0);
}

void TextureArray2D::createBlkTextureArrayImageView() {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

  viewInfo.image = blkTextureArrayImage;

  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

  viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB; // or your chosen format

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;

  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = this->layerCount;

  if (vkCreateImageView(vkContext.device, &viewInfo, nullptr,
                        &blkTextureArrayImageView) != VK_SUCCESS) {
    throw std::runtime_error(
        "failed to create block texture array image view!");
  }
}

void TextureArray2D::transitionBlkTextureArrayImageLayout() {
  VkCommandBuffer cmdBuffer = beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = blkTextureArrayImage;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = this->layerCount;

  // ONLY transition to TRANSFER_DST here — upload happens after
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

  vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  endSingleTimeCommands(cmdBuffer);
}

void TextureArray2D::createBlkTextureStagingBuffer() {
  const VkDeviceSize texSize = textureWidth * textureHeight * 4;
  const VkDeviceSize bufferSize = texSize * this->layerCount;

  // 1. Create buffer
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkCreateBuffer(vkContext.device, &bufferInfo, nullptr, &stagingBlkBuffer);

  // 2. Get memory requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(vkContext.device, stagingBlkBuffer,
                                &memRequirements);

  // 3. Allocate host-visible memory
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  vkAllocateMemory(vkContext.device, &allocInfo, nullptr,
                   &stagingBlkBufferMemory);

  // 4. Bind memory
  vkBindBufferMemory(vkContext.device, stagingBlkBuffer, stagingBlkBufferMemory,
                     0);
}

void TextureArray2D::createBlkArraySampler() {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_NEAREST;
  samplerInfo.minFilter = VK_FILTER_NEAREST;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1.f;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  samplerInfo.minLod = 0.f;
  samplerInfo.maxLod = 0.f;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  if (vkCreateSampler(vkContext.device, &samplerInfo, nullptr,
                      &blkArraySampler) != VK_SUCCESS)
    throw std::runtime_error("failed to create block array sampler!");
}

void TextureArray2D::setTexturePaths(uint32_t idx, std::string path) {
  this->filePaths.at(idx) = path;
}

void TextureArray2D::allocateStorageForTextures() {

  for (int i = 0; i < layerCount; i++) {
    int texWidth, texHeight, texChannels;

    stbi_uc *pixels = stbi_load(filePaths[i].c_str(), &texWidth, &texHeight,
                                &texChannels, STBI_rgb_alpha);

    if (!pixels) {
      throw std::runtime_error("Failed to load image");
    }

    // Allocate storage
    blockTextures[i] = new unsigned char[textureWidth * textureHeight * 4];

    // Copy into our array slot
    memcpy(blockTextures[i], pixels, textureWidth * textureHeight * 4);

    stbi_image_free(pixels);
  }
}

void TextureArray2D::uploadBlkTexturesToArray() {
  const uint32_t TEX_COUNT = this->layerCount;
  const uint32_t TEX_W = textureWidth;
  const uint32_t TEX_H = textureHeight;
  const uint32_t CHANNELS = 4;

  const VkDeviceSize texSize = TEX_W * TEX_H * CHANNELS;
  const VkDeviceSize imageSize = texSize * TEX_COUNT;

  // 1. Map staging memory
  void *data = nullptr;

  vkMapMemory(vkContext.device, stagingBlkBufferMemory, 0, imageSize, 0, &data);

  // 2. Copy all textures into one contiguous buffer
  for (uint32_t i = 0; i < TEX_COUNT; i++) {
    memcpy(static_cast<char *>(data) + (i * texSize), blockTextures[i],
           texSize);
  }

  vkUnmapMemory(vkContext.device, stagingBlkBufferMemory);

  // 3. Record copy command
  VkCommandBuffer cmd = beginSingleTimeCommands();

  std::vector<VkBufferImageCopy> regions;
  regions.reserve(TEX_COUNT);

  for (uint32_t i = 0; i < TEX_COUNT; i++) {
    VkBufferImageCopy region{};

    region.bufferOffset = i * texSize;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = i;
    region.imageSubresource.layerCount = 1;

    region.imageExtent = {TEX_W, TEX_H, 1};

    regions.push_back(region);
  }

  vkCmdCopyBufferToImage(cmd, stagingBlkBuffer, blkTextureArrayImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         static_cast<uint32_t>(regions.size()), regions.data());

  endSingleTimeCommands(cmd);
}

void TextureArray2D::transitionBlkTextureArrayToShaderRead() {
  VkCommandBuffer cmd = beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = blkTextureArrayImage;

  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = this->layerCount;

  // IMPORTANT: synchronization masks
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  endSingleTimeCommands(cmd);
}
void TextureArray2D::cleanUp() {
    blockTextures.clear();
    filePaths.clear();

    if (stagingBlkBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkContext.device, stagingBlkBuffer, nullptr);
        vkFreeMemory(vkContext.device, stagingBlkBufferMemory, nullptr);
        stagingBlkBuffer = VK_NULL_HANDLE;          // add this
        stagingBlkBufferMemory = VK_NULL_HANDLE;    // add this
    }
    if (blkTextureArrayImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(vkContext.device, blkTextureArrayImageView, nullptr);
        blkTextureArrayImageView = VK_NULL_HANDLE;
    }
    if (blkTextureArrayImage != VK_NULL_HANDLE) {
        vkDestroyImage(vkContext.device, blkTextureArrayImage, nullptr);
        blkTextureArrayImage = VK_NULL_HANDLE;
    }
    if (blkTextureArrayMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vkContext.device, blkTextureArrayMemory, nullptr);
        blkTextureArrayMemory = VK_NULL_HANDLE;     // add this
    }
    if (blkArraySampler != VK_NULL_HANDLE) {
        vkDestroySampler(vkContext.device, blkArraySampler, nullptr);
        blkArraySampler = VK_NULL_HANDLE;           // add this
    }
}
TextureArray2D::~TextureArray2D() {}
