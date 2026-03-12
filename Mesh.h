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
    void createVertexBuffer();
    void createIndexBuffer();

    void cleanup();
    ~Mesh();
};