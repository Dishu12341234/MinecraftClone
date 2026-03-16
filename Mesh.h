#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include "GraphicsPipeline.h"

class Mesh
{
private:
    VulkanContext &vkContext;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;

    VkBuffer StagingVertexBuffer = VK_NULL_HANDLE;
    VkBuffer StagingIndexBuffer = VK_NULL_HANDLE;

    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

    VkDeviceMemory StagingVertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory StagingIndexBufferMemory = VK_NULL_HANDLE;

    VkDeviceSize stagingVertexBufferSize = 0;
    VkDeviceSize stagingIndexBufferSize = 0;

    void *stagingIndexBufferData;
    void *stagingVertexBufferData;

    friend class Chunk;

public:
    Mesh(VulkanContext &vkContext);

    void loadVertices(const std::vector<Vertex> &vertices);
    void loadIndices(const std::vector<uint32_t> &indices);

    void createVertexBuffer();
    void createIndexBuffer();

    void updateVertexBuffer();
    void updateIndexBuffer();

    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
              std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 &c1);

    void cleanup();
    ~Mesh();
};