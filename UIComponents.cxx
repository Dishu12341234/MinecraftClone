#include "UIComponents.h"
#include "utils.h"
#include <cstring>

UIComponents::UIComponents(VulkanContext &vkContext) : vkContext{vkContext}
{
}
void UIComponents::createVertexBuffer()
{
    stagingVertexBufferSize = sizeof(vertices[0]) * vertices.size();

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

void UIComponents::createIndexBuffer()
{
    stagingIndexBufferSize = sizeof(indices[0]) * indices.size();

    if (stagingIndexBufferSize == 0)
        return;

    utils::createBuffer(stagingIndexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingIndexBuffer, StagingIndexBufferMemory, vkContext);

    vkMapMemory(vkContext.device, StagingIndexBufferMemory, 0, stagingIndexBufferSize, 0, &stagingIndexBufferData);
    memcpy(stagingIndexBufferData, indices.data(), (size_t)stagingIndexBufferSize);

    utils::createBuffer(stagingIndexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, vkContext);

    utils::copyBuffer(StagingIndexBuffer, indexBuffer, stagingIndexBufferSize, vkContext);
}

void UIComponents::initUIComponent(glm::vec2 position, glm::vec2 size)
{
    vertices.clear();
    indices.clear();
    vertices.reserve(4);
    indices.reserve(6);

    this->position = position;
    this->size = size;

    float cx = position.x - size.x / 2; // also fix this, was missing /2
    float cy = position.y - size.y / 2;
    float w = size.x, h = size.y;

    vertices = {
        {{cx, cy, 0.0f}, {0.0f, 0.0f}, (uint32_t)textureIDX},         // bottom-left
        {{cx, cy + h, 0.0f}, {0.0f, 1.0f}, (uint32_t)textureIDX},     // top-left
        {{cx + w, cy + h, 0.0f}, {1.0f, 1.0f}, (uint32_t)textureIDX}, // top-right
        {{cx + w, cy, 0.0f}, {1.0f, 0.0f}, (uint32_t)textureIDX},     // bottom-right
    };

    indices = {0, 1, 2, 2, 3, 0};

    createVertexBuffer();
    createIndexBuffer();
}

void UIComponents::setTextureIDX(int idx)
{
    this->textureIDX = idx;
}

void UIComponents::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 &c1)
{
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};

    if (vertices.size() == 0 || indices.size() == 0) [[unlikely]]
        return;

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantC1), &c1);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &(descriptorSets[currentFrame]), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void UIComponents::cleanup()
{
    if (StagingVertexBufferMemory != VK_NULL_HANDLE)
        vkUnmapMemory(vkContext.device, StagingVertexBufferMemory);
    if (StagingIndexBufferMemory != VK_NULL_HANDLE)
        vkUnmapMemory(vkContext.device, StagingIndexBufferMemory);

    if (indexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vkContext.device, indexBuffer, nullptr);
        vkFreeMemory(vkContext.device, indexBufferMemory, nullptr);
    }
    if (vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vkContext.device, vertexBuffer, nullptr);
        vkFreeMemory(vkContext.device, vertexBufferMemory, nullptr);
    }
    if (StagingIndexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vkContext.device, StagingIndexBuffer, nullptr);
        vkFreeMemory(vkContext.device, StagingIndexBufferMemory, nullptr);
    }
    if (StagingVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vkContext.device, StagingVertexBuffer, nullptr);
        vkFreeMemory(vkContext.device, StagingVertexBufferMemory, nullptr);
    }
}

UIComponents::~UIComponents()
{
}
