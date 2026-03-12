#include "MeshUploader.h"
#include <cstring>

uint32_t findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkCommandBuffer beginSingleTimeCommands(
    VkDevice device,
    VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void endSingleTimeCommands(
    VkDevice device,
    VkQueue graphicsQueue,
    VkCommandPool commandPool,
    VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void MeshUploader::upload(
    VulkanContext vkContext,
    GameMeshObject &mesh)
{

    mesh.vkContext = vkContext;

    VkDeviceSize vertexSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();
    VkDeviceSize indexSize = sizeof(mesh.indices[0]) * mesh.indices.size();

    // === 1. Create staging buffer ===
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = vertexSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(vkContext.device, &bufferInfo, nullptr, &stagingBuffer);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(vkContext.device, stagingBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        vkContext.physicalDevice,
        memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(vkContext.device, &allocInfo, nullptr, &stagingMemory);
    vkBindBufferMemory(vkContext.device, stagingBuffer, stagingMemory, 0);

    // Copy vertex data
    void *data;
    vkMapMemory(vkContext.device, stagingMemory, 0, vertexSize, 0, &data);
    memcpy(data, mesh.vertices.data(), (size_t)vertexSize);
    vkUnmapMemory(vkContext.device, stagingMemory);

    // === 2. Create device-local vertex buffer ===
    bufferInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vkCreateBuffer(vkContext.device, &bufferInfo, nullptr, &mesh.vertexBuffer);
    vkGetBufferMemoryRequirements(vkContext.device, mesh.vertexBuffer, &memReq);

    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        vkContext.physicalDevice,
        memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(vkContext.device, &allocInfo, nullptr, &mesh.vertexBufferMemory);
    vkBindBufferMemory(vkContext.device, mesh.vertexBuffer, mesh.vertexBufferMemory, 0);

    // === 3. Copy staging → vertex buffer ===
    VkCommandBuffer cmd = beginSingleTimeCommands(vkContext.device, vkContext.commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = vertexSize;
    vkCmdCopyBuffer(cmd, stagingBuffer, mesh.vertexBuffer, 1, &copyRegion);

    endSingleTimeCommands(vkContext.device, vkContext.graphicsQueue, vkContext.commandPool, cmd);

    // === 4. Cleanup staging ===
    vkDestroyBuffer(vkContext.device, stagingBuffer, nullptr);
    vkFreeMemory(vkContext.device, stagingMemory, nullptr);

    // ================= INDEX BUFFER =================

    if (!mesh.indices.empty())
    {
        VkBuffer indexStagingBuffer;
        VkDeviceMemory indexStagingMemory;

        VkBufferCreateInfo indexBufferInfo{};
        indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexBufferInfo.size = indexSize;
        indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateBuffer(vkContext.device, &indexBufferInfo, nullptr, &indexStagingBuffer);

        VkMemoryRequirements indexMemReq;
        vkGetBufferMemoryRequirements(vkContext.device, indexStagingBuffer, &indexMemReq);

        VkMemoryAllocateInfo indexAllocInfo{};
        indexAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        indexAllocInfo.allocationSize = indexMemReq.size;
        indexAllocInfo.memoryTypeIndex = findMemoryType(
            vkContext.physicalDevice,
            indexMemReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(vkContext.device, &indexAllocInfo, nullptr, &indexStagingMemory);
        vkBindBufferMemory(vkContext.device, indexStagingBuffer, indexStagingMemory, 0);

        // copy index data
        void *indexData;
        vkMapMemory(vkContext.device, indexStagingMemory, 0, indexSize, 0, &indexData);
        memcpy(indexData, mesh.indices.data(), (size_t)indexSize);
        vkUnmapMemory(vkContext.device, indexStagingMemory);

        // device-local index buffer
        indexBufferInfo.usage =
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        vkCreateBuffer(vkContext.device, &indexBufferInfo, nullptr, &mesh.indexBuffer);
        vkGetBufferMemoryRequirements(vkContext.device, mesh.indexBuffer, &indexMemReq);

        indexAllocInfo.allocationSize = indexMemReq.size;
        indexAllocInfo.memoryTypeIndex = findMemoryType(
            vkContext.physicalDevice,
            indexMemReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(vkContext.device, &indexAllocInfo, nullptr, &mesh.indexBufferMemory);
        vkBindBufferMemory(vkContext.device, mesh.indexBuffer, mesh.indexBufferMemory, 0);

        // copy staging → device
        VkCommandBuffer indexCmd =
            beginSingleTimeCommands(vkContext.device, vkContext.commandPool);

        VkBufferCopy indexCopy{};
        indexCopy.size = indexSize;
        vkCmdCopyBuffer(indexCmd, indexStagingBuffer, mesh.indexBuffer, 1, &indexCopy);

        endSingleTimeCommands(
            vkContext.device,
            vkContext.graphicsQueue,
            vkContext.commandPool,
            indexCmd);

        // cleanup
        vkDestroyBuffer(vkContext.device, indexStagingBuffer, nullptr);
        vkFreeMemory(vkContext.device, indexStagingMemory, nullptr);
    }
}
