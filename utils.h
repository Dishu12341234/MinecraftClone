#pragma once
#include "PassInfo.hpp"
#include "Structs.h"

namespace utils
{
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory, VulkanContext &vkContext);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VulkanContext &vkContext);
    inline int signum(int x)
    {
        if (x > 0)
        {
            return 1;
        }
        else if (x < 0)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }

} // namespace utils
