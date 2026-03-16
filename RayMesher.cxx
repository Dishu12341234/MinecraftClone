#include "RayMesher.h"
#include "utils.h"
#include <cstring>

RayMesher::RayMesher(VulkanContext &vkContext) : vkContext{vkContext}
{
}

void RayMesher::loadRayPoints(const std::vector<RayPoints> &vertices)
{
    this->vertices = vertices;
}

void RayMesher::loadIndices(const std::vector<uint32_t> &indices)
{
    this->indices = indices;
}

void RayMesher::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    if (bufferSize == 0)
        return;

    // TX
    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, StagingVertexBuffer, StagingVertexBufferMemory, vkContext);

    // filling in the vertex buffer
    vkMapMemory(vkContext.device, StagingVertexBufferMemory, 0, bufferSize, 0, &stagingVertexBufferData);
    memcpy(stagingVertexBufferData, vertices.data(), (size_t)bufferSize);

    // creating the actul VBO and setting it up so the it is the destionation of the transfer source
    // TX
    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory, vkContext);

    // but we still have to copy the memory contents manully
    utils::copyBuffer(StagingVertexBuffer, vertexBuffer, bufferSize, vkContext);
}

void RayMesher::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    if (bufferSize == 0)
        return;

    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, StagingIndexBuffer, StagingIndexBufferMemory, vkContext);

    vkMapMemory(vkContext.device, StagingIndexBufferMemory, 0, bufferSize, 0, &stagingIndexBufferData);
    memcpy(stagingIndexBufferData, indices.data(), (size_t)bufferSize);

    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, vkContext);

    utils::copyBuffer(StagingIndexBuffer, indexBuffer, bufferSize, vkContext);
}

void RayMesher::updateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(RayPoints) * vertices.size();

    if (bufferSize == 0)
        return;

    memcpy(stagingVertexBufferData, vertices.data(), (size_t)bufferSize);
    utils::copyBuffer(StagingVertexBuffer, vertexBuffer, bufferSize, vkContext);
}

void RayMesher::updateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

    if (bufferSize == 0)
        return;

    memcpy(stagingIndexBufferData, indices.data(), (size_t)bufferSize);
    utils::copyBuffer(StagingIndexBuffer, indexBuffer, bufferSize, vkContext);
}

void RayMesher::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 &c1)
{
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};

    [[unlikely]] if (vertices.size() == 0 || indices.size() == 0)
        return;

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantC1), &c1);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &(descriptorSets[currentFrame]), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void RayMesher::cleanup()
{
    vkDestroyBuffer(vkContext.device, indexBuffer, nullptr);
    vkFreeMemory(vkContext.device, indexBufferMemory, nullptr);

    vkDestroyBuffer(vkContext.device, vertexBuffer, nullptr);
    vkFreeMemory(vkContext.device, vertexBufferMemory, nullptr);

    vkDestroyBuffer(vkContext.device, StagingIndexBuffer, nullptr);
    vkFreeMemory(vkContext.device, StagingIndexBufferMemory, nullptr);

    vkDestroyBuffer(vkContext.device, StagingVertexBuffer, nullptr);
    vkFreeMemory(vkContext.device, StagingVertexBufferMemory, nullptr);
}

RayMesher::~RayMesher()
{
}
