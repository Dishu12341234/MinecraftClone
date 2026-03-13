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
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    friend class Chunk;

public:
    Mesh(VulkanContext &vkContext);

    void loadVertices(const std::vector<Vertex> &vertices);
    void loadIndices(const std::vector<uint32_t> &indices);

    void createVertexBuffer();
    void createIndexBuffer();

    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
                 std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent, PushConstantC1 &c1);

    void cleanup();
    ~Mesh();
};