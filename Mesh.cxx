#include "Mesh.h"
#include "utils.h"
#include <iostream>
#include <cstring>

Mesh::Mesh(VulkanContext &vkContext) : vkContext{vkContext}
{
}

void Mesh::loadVertices(const std::vector<Vertex> &vertices)
{
    this->vertices = vertices;
}

void Mesh::loadIndices(const std::vector<uint32_t> &indices)
{
    this->indices = indices;
}

void Mesh::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    if (bufferSize == 0)
        return;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // TX
    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, stagingBuffer, stagingBufferMemory, vkContext);

    // filling in the vertex buffer
    void *data;
    vkMapMemory(vkContext.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(vkContext.device, stagingBufferMemory);

    // creating the actul VBO and setting it up so the it is the destionation of the transfer source
    // TX
    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory, vkContext);

    // but we still have to copy the memory contents manully
    utils::copyBuffer(stagingBuffer, vertexBuffer, bufferSize, vkContext);

    vkDestroyBuffer(vkContext.device, stagingBuffer, nullptr);
    vkFreeMemory(vkContext.device, stagingBufferMemory, nullptr);
}

void Mesh::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    if (bufferSize == 0)
        return;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, vkContext);

    void *data;
    vkMapMemory(vkContext.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(vkContext.device, stagingBufferMemory);

    utils::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, vkContext);

    utils::copyBuffer(stagingBuffer, indexBuffer, bufferSize, vkContext);

    vkDestroyBuffer(vkContext.device, stagingBuffer, nullptr);
    vkFreeMemory(vkContext.device, stagingBufferMemory, nullptr);
}

void Mesh::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 &c1)
{
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};

    [[likely]] if (vertices.size() == 0 || indices.size() == 0)
        return;

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantC1), &c1);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &(descriptorSets[currentFrame]), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Mesh::cleanup()
{
    vkDestroyBuffer(vkContext.device, indexBuffer, nullptr);
    vkFreeMemory(vkContext.device, indexBufferMemory, nullptr);

    vkDestroyBuffer(vkContext.device, vertexBuffer, nullptr);
    vkFreeMemory(vkContext.device, vertexBufferMemory, nullptr);
}

Mesh::~Mesh()
{
}
