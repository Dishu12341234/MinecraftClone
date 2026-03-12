#pragma once
#include "PassInfo.hpp"
#include "Structs.h"

namespace utils
{
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory, VulkanContext &vkContext);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VulkanContext &vkContext);
} // namespace utils
