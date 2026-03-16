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
    stagingVertexBufferSize = sizeof(vertices[0]) * vertices.size() * 2;

    std::cout << "Size of staging VBO " << stagingVertexBufferSize << std::endl;

    if (stagingVertexBufferSize == 0)
        return;

    // TX
    utils::createBuffer(stagingVertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingVertexBuffer, StagingVertexBufferMemory, vkContext);

    // filling in the vertex buffer
    vkMapMemory(vkContext.device, StagingVertexBufferMemory, 0, stagingVertexBufferSize, 0, &stagingVertexBufferData);
    memcpy(stagingVertexBufferData, vertices.data(), (size_t)stagingVertexBufferSize);

    // creating the actul VBO and setting it up so the it is the destionation of the transfer source
    // TX
    utils::createBuffer(stagingVertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory, vkContext);

    // but we still have to copy the memory contents manully
    utils::copyBuffer(StagingVertexBuffer, vertexBuffer, stagingVertexBufferSize, vkContext);
}

void Mesh::createIndexBuffer()
{
    stagingIndexBufferSize = sizeof(indices[0]) * indices.size() * 2;

    if (stagingIndexBufferSize == 0)
        return;

    utils::createBuffer(stagingIndexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingIndexBuffer, StagingIndexBufferMemory, vkContext);

    vkMapMemory(vkContext.device, StagingIndexBufferMemory, 0, stagingIndexBufferSize, 0, &stagingIndexBufferData);
    memcpy(stagingIndexBufferData, indices.data(), (size_t)stagingIndexBufferSize);

    utils::createBuffer(stagingIndexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, vkContext);

    utils::copyBuffer(StagingIndexBuffer, indexBuffer, stagingIndexBufferSize, vkContext);
}

void Mesh::updateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    std::cout << "Updating Vertex Buffer..." << std::endl;

    std::cout << "required buffer Size: " << bufferSize << " bytes" << std::endl;
    std::cout << "current buffer size: " << stagingVertexBufferSize << " bytes" << std::endl;

    if (bufferSize == 0)
        return;

    memcpy(stagingVertexBufferData, vertices.data(), (size_t)bufferSize);
    utils::copyBuffer(StagingVertexBuffer, vertexBuffer, bufferSize, vkContext);
}

void Mesh::updateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    if (bufferSize == 0)
        return;

    memcpy(stagingIndexBufferData, indices.data(), (size_t)bufferSize);
    utils::copyBuffer(StagingIndexBuffer, indexBuffer, bufferSize, vkContext);
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
    if (StagingVertexBufferMemory != VK_NULL_HANDLE)
        vkUnmapMemory(vkContext.device, StagingVertexBufferMemory);  // missing from your code!
    if (StagingIndexBufferMemory != VK_NULL_HANDLE)
        vkUnmapMemory(vkContext.device, StagingIndexBufferMemory);   // missing too!

    if (indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkContext.device, indexBuffer, nullptr);
        vkFreeMemory(vkContext.device, indexBufferMemory, nullptr);
    }
    if (vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkContext.device, vertexBuffer, nullptr);
        vkFreeMemory(vkContext.device, vertexBufferMemory, nullptr);
    }
    if (StagingIndexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkContext.device, StagingIndexBuffer, nullptr);
        vkFreeMemory(vkContext.device, StagingIndexBufferMemory, nullptr);
    }
    if (StagingVertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkContext.device, StagingVertexBuffer, nullptr);
        vkFreeMemory(vkContext.device, StagingVertexBufferMemory, nullptr);
    }
}

Mesh::~Mesh()
{
}
